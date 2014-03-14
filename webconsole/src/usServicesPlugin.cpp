/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "usServicesPlugin_p.h"

#include "usHttpServletRequest.h"
#include "usHttpServletResponse.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include <set>

US_BEGIN_NAMESPACE

static std::string LABEL()
{
  static std::string s = "services";
  return s;
}

static std::string TITLE()
{
  static std::string s = "CppMicroServices Services";
  return s;
}

static std::string CATEGORY()
{
  static std::string s;
  return s;
}

ServicesPlugin::ServicesPlugin()
  : SimpleWebConsolePlugin(LABEL(), TITLE(), CATEGORY())
  , m_TemplateRS(NULL)
  , m_TemplateSI(NULL)
{
}

ServicesPlugin::~ServicesPlugin()
{
  delete m_TemplateRS;
  delete m_TemplateSI;
}

bool ServicesPlugin::IsHtmlRequest(HttpServletRequest& request)
{
  std::vector<std::string> acceptHeaders = request.GetHeaders("Accept");
  std::vector<std::string>::iterator htmlIter = std::find(acceptHeaders.begin(), acceptHeaders.end(), "text/html");
  std::vector<std::string>::iterator jsonIter = std::find(acceptHeaders.begin(), acceptHeaders.end(), "application/json");
  if (jsonIter < htmlIter)
  {
    request.SetAttribute("_format", std::string("json"));
    return false;
  }
  else
  {
    request.SetAttribute("_format", std::string("html"));
    return true;
  }
}

void ServicesPlugin::RenderContent(HttpServletRequest& request, HttpServletResponse& response)
{
  std::string pathInfo = request.GetPathInfo();
  std::string format = request.GetAttribute("_format").ToString();
  if (pathInfo == "/services")
  {
    if (format == "json")
    {
      response.SetContentType("application/json");
      response.GetOutputStream() << GetIds_JSON();
    }
    else
    {
      if (m_TemplateRS == NULL)
      {
        ModuleResource res = GetModuleContext()->GetModule()->GetResource("/templates/services.html");
        m_TemplateRS = new ModuleResourceStream(res, std::ios_base::binary);
      }
      m_TemplateRS->seekg(0, std::ios_base::beg);
      response.GetOutputStream() << m_TemplateRS->rdbuf();
    }
  }
  else if (pathInfo.size() > 20 && pathInfo.compare(0, 20, "/services/interface/") == 0)
  {
    std::string id = pathInfo.substr(20);
    if (format == "json")
    {
      response.SetContentType("application/json");
      response.GetOutputStream() << GetInterface_JSON(id);
    }
    else
    {
      if (m_TemplateSI == NULL)
      {
        ModuleResource res = GetModuleContext()->GetModule()->GetResource("/templates/service_interface.html");
        m_TemplateSI = new ModuleResourceStream(res, std::ios_base::binary);
      }
      m_TemplateSI->seekg(0, std::ios_base::beg);
      response.GetOutputStream() << m_TemplateSI->rdbuf();
    }
  }
  else
  {

  }
}

std::string ServicesPlugin::GetIds_JSON() const
{
  std::set<std::string> ids;
  std::vector<ServiceReferenceU> refs = GetContext()->GetServiceReferences("");
  for (std::vector<ServiceReferenceU>::const_iterator iter = refs.begin(), endIter = refs.end();
       iter != endIter; ++iter)
  {
    Any objectClass = iter->GetProperty(ServiceConstants::OBJECTCLASS());
    std::vector<std::string>& oc = ref_any_cast<std::vector<std::string> >(objectClass);
    for (std::vector<std::string>::const_iterator ocIter = oc.begin(), ocEndIter = oc.end();
         ocIter != ocEndIter; ++ocIter)
    {
      ids.insert(*ocIter);
    }
  }

  std::string json = "{ \"ids\" : [";
  for (std::set<std::string>::const_iterator iter = ids.begin(), endIter = ids.end();
       iter != endIter; ++iter)
  {
    if (iter == ids.begin())
    {
      json += "\"" + *iter + "\"";
    }
    else
    {
      json += ", \"" + *iter + "\"";
    }
  }
  json += "]}";
  return json;
}

std::string ServicesPlugin::GetInterface_JSON(const std::string& iid) const
{
  std::vector<ServiceReferenceU> refs = GetContext()->GetServiceReferences(iid);
  std::stringstream json;
  json << "{ \"services\" : [";
  for (std::vector<ServiceReferenceU>::const_iterator iter = refs.begin(), endIter = refs.end();
       iter != endIter; ++iter)
  {
    std::vector<std::string> keys;
    iter->GetPropertyKeys(keys);
    Any propsAny = std::map<std::string, Any>();
    std::map<std::string, Any>& props = ref_any_cast<std::map<std::string, Any> >(propsAny);
    for (std::vector<std::string>::const_iterator keyIter = keys.begin(), keyEndIter = keys.end();
         keyIter != keyEndIter; ++keyIter)
    {
      if (*keyIter != ServiceConstants::SERVICE_ID() &&
          *keyIter != ServiceConstants::SERVICE_RANKING() &&
          *keyIter != ServiceConstants::OBJECTCLASS() &&
          *keyIter != ServiceConstants::SERVICE_SCOPE())
      {
        props.insert(std::make_pair(*keyIter, iter->GetProperty(*keyIter)));
      }
    }

    if (iter != refs.begin())
    {
      json << ",";
    }
    json << "{ \"module\":\"" << iter->GetModule()->GetName() << "\","
         << "  \"id\":" << iter->GetProperty(ServiceConstants::SERVICE_ID()).ToJSON() << ","
         << "  \"ranking\":" << iter->GetProperty(ServiceConstants::SERVICE_RANKING()).ToJSON() << ","
         << "  \"scope\":" << iter->GetProperty(ServiceConstants::SERVICE_SCOPE()).ToJSON() << ","
         << "  \"classes\":" << iter->GetProperty(ServiceConstants::OBJECTCLASS()).ToJSON() << ","
         << "  \"props\":" << propsAny.ToJSON()
         << "}";
  }
  json << "]}";
  return json.str();
}

US_END_NAMESPACE
