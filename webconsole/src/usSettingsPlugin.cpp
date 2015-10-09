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

#include "usSettingsPlugin_p.h"

#include "usHttpServletRequest.h"
#include "usHttpServletResponse.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

US_BEGIN_NAMESPACE

static std::string LABEL()
{
  static std::string s = "settings";
  return s;
}

static std::string TITLE()
{
  static std::string s = "CppMicroServices Settings";
  return s;
}

static std::string CATEGORY()
{
  static std::string s;
  return s;
}

SettingsPlugin::SettingsPlugin()
  : SimpleWebConsolePlugin(LABEL(), TITLE(), CATEGORY())
  , m_TemplateRS(NULL)
{
}

SettingsPlugin::~SettingsPlugin()
{
  delete m_TemplateRS;
}

void SettingsPlugin::RenderContent(HttpServletRequest&, HttpServletResponse& response)
{
  if (m_TemplateRS == NULL)
  {
    ModuleResource res = GetModuleContext()->GetModule()->GetResource("/templates/settings.html");
    m_TemplateRS = new ModuleResourceStream(res, std::ios_base::binary);
  }
  m_TemplateRS->seekg(0, std::ios_base::beg);
  response.GetOutputStream() << m_TemplateRS->rdbuf();
}

ModuleResource SettingsPlugin::GetResource(const std::string& path) const
{
  return (this->GetContext() != NULL) ?
        this->GetContext()->GetModule()->GetResource(path) :
        ModuleResource();
}

/*
cpptempl::data_map& SettingsPlugin::getData()
{
  m_Data.clear();
  m_Data["mitk-version"] = cpptempl::make_data(MITK_VERSION_STRING);
  m_Data["mitk-githash"] = cpptempl::make_data(MITK_REVISION);

  m_Data["us-thread"] = cpptempl::make_data(us::ModuleSettings::IsThreadingSupportEnabled() ? "true" : "false");
  m_Data["us-autoload"] = cpptempl::make_data(us::ModuleSettings::IsAutoLoadingEnabled() ? "true" : "false");
  m_Data["us-storagepath"] = cpptempl::make_data(us::ModuleSettings::GetStoragePath());

  cpptempl::data_list autoLoadPaths;
  us::ModuleSettings::PathList pathList = us::ModuleSettings::GetAutoLoadPaths();
  for (us::ModuleSettings::PathList::iterator iter = pathList.begin(),
       endIter = pathList.end(); iter != endIter; ++iter)
  {
    autoLoadPaths.push_back(cpptempl::make_data(*iter));
  }
  m_Data["us-autoload-paths"] = cpptempl::make_data(autoLoadPaths);

  return m_Data;
}
*/

US_END_NAMESPACE
