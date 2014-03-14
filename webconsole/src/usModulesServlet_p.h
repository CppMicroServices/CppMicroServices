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

#ifndef USMODULESSERVLET_P_H
#define USMODULESSERVLET_P_H

#include "usSimpleWebConsolePlugin.h"

US_BEGIN_NAMESPACE

class ModulesServlet : public SimpleWebConsolePlugin
{
  ModulesHtml(us::ModuleContext* context)
    : ConsoleHtmlHandler(context)
  {}

private:

  virtual bool sendHtml(struct mg_connection *conn, const std::string& uri)
  {
    std::string resourceUri = getResourceUri(uri);
    if (resourceUri.empty())
    {
      return false;
    }

    us::ModuleResource resource = m_Context->GetModule()->GetResource(resourceUri);
    if (!resource)
    {
      MBI_INFO << "Resource " << resourceUri << " not found";
      return false;
    }

    us::ModuleResourceStream resourceStream(resource, std::ios::binary);
    std::string result;
    resourceStream.seekg(0, std::ios::end);
    result.resize(resourceStream.tellg());
    resourceStream.seekg(0, std::ios::beg);
    resourceStream.read(&result[0], result.size());

    sendHtmlHeader(conn, resourceUri, result.size() + m_HtmlFooterTemplate.size());
    mg_write(conn, result.c_str(), result.size());
    mg_write(conn, m_HtmlFooterTemplate.c_str(), m_HtmlFooterTemplate.size());

    return true;
  }

    virtual bool sendJson(struct mg_connection *conn, const std::string& uri)
    {
      std::string json;
      if (uri == "/Console/modules")
      {
        json += "{ \"modules\": [";
        std::vector<us::Module*> modules = m_Context->GetModules();
        for(std::vector<us::Module*>::iterator iter = modules.begin(), endIter = modules.end();
            iter != endIter; ++iter)
        {
          if (iter != modules.begin())
          {
            json += ",";
          }
          json += "{\n  \"id\": " + cpptempl::lexical_cast((*iter)->GetModuleId()) + ",\n";
          json += "  \"name\": \"" + (*iter)->GetName() + "\",\n";
          us::Any description = (*iter)->GetProperty(us::Module::PROP_DESCRIPTION());
          json += "  \"description\": \"" + (description.Empty() ? std::string() : description.ToString()) + "\",\n";
          json += "  \"version\": \"" + (*iter)->GetVersion().ToString() + "\",\n";
          us::Any vendor = (*iter)->GetProperty(us::Module::PROP_VENDOR());
          json += "  \"vendor\": \"" + (vendor.Empty() ? std::string() : vendor.ToString()) + "\",\n";
          json += "  \"loaded\": " + ((*iter)->IsLoaded() ? std::string("true") : std::string("false")) + "\n}\n";
        }
        json += "]}";
      }
      else
      {
        return false;
      }

      mg_printf(conn,
                "HTTP/1.1 200 OK\r\n"
                /* "Cache: no-cache\r\n" */
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "\r\n",
                "application/json", json.size());
      mg_write(conn, json.c_str(), json.size());

      return true;
    }

  virtual bool sendJson(struct mg_connection* conn, const std::string& uri)
  {
    std::size_t pos = uri.find_last_of('/');
    if (pos == std::string::npos) return false;
    if (uri.substr(0, pos) != "/Console/modules" || pos == uri.size() - 1) return false;
    std::string strId = uri.substr(pos+1);
    if (strId.empty()) return false;
    long int id = -1;
    std::istringstream(strId) >> id;
    if (id < 0) return false;

    us::Module* module = us::ModuleRegistry::GetModule(id);
    if (module == NULL) return false;

    // ----------------- Get module properties ---------------------

    std::string json = "{ \"id\": " + strId + ",\n\"name\":\"" + module->GetName() + "\",\n\"properties\": [\n";
    std::vector<std::string> keys = module->GetPropertyKeys();
    bool separatorNeeded = false;
    for (std::vector<std::string>::iterator iter = keys.begin(),
         endIter = keys.end(); iter != endIter; ++iter)
    {
      us::Any value = module->GetProperty(*iter);
      if (!value.Empty())
      {
        if (separatorNeeded) json += ",";
        else separatorNeeded = true;
        json += "{ \"key\":\"" + *iter + "\", \"value\":\"" + value.ToString() + "\"}\n";
      }
    }
    json += "]";

    // --------------- Get module resource information ------------------

    us::ModuleResource resource = module->GetResource("/");
    if (resource.IsValid())
    {
      json += ",\n\"resources\":\n";
      GetJsonRepresentation(module, resource, json, 0);
    }

    // ------------- Get registered service information ----------------
    std::vector<us::ServiceReferenceU> registeredServices = module->GetRegisteredServices();
    if (!registeredServices.empty())
    {
      json += ",\n\"registered_services\": [";
      for (std::vector<us::ServiceReferenceU>::iterator iter = registeredServices.begin(),
           endIter = registeredServices.end(); iter != endIter; ++iter)
      {

      }
    }

    json += "}";

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              /* "Cache: no-cache\r\n" */
              "Content-Type: %s\r\n"
              "Content-Length: %ld\r\n"
              "\r\n",
              "application/json", json.size());
    mg_write(conn, json.c_str(), json.size());
    return true;
  }

  void GetJsonRepresentation(us::Module* module, const us::ModuleResource& currResource, std::string& json, int level)
  {
    std::string indent;
    for (int i = 0 ; i < level; i++)
    {
      indent += "    ";
    }

    if (currResource.IsFile())
    {
      json += indent + "{ \"label\":\"" + currResource.GetName() + "\",\n" +
              indent + "  \"size\":" + cpptempl::lexical_cast(currResource.GetSize()) + ",\n" +
              indent + "  \"href\":\"/Console/resources/" + cpptempl::lexical_cast(module->GetModuleId()) + currResource.GetResourcePath() + "\"\n" +
              /*indent + "  \"children\":null*/ "}";
    }
    else
    {
      json += indent + "{ \"label\":\"" + (currResource.GetName().empty() ? currResource.GetResourcePath() : currResource.GetName()) + "\",\n" +
              indent + "  \"size\":null\n";
      std::vector<std::string> children = currResource.GetChildren();
      if (children.empty())
      {
        json += /*indent + "  \"children\":null*/ "}";
      }
      else
      {
        json += indent + ",  \"children\": [\n";
        std::cout << "Children for " << currResource.GetResourcePath() << ":" << std::endl;
        std::vector<us::ModuleResource> childDirs;
        std::vector<us::ModuleResource> childFiles;
        for (std::vector<std::string>::iterator iter = children.begin(),
             endIter = children.end(); iter != endIter; ++iter)
        {
          us::ModuleResource childResource = module->GetResource(currResource.GetResourcePath() + "/" + *iter);
          if (childResource.IsValid())
          {
            if (childResource.IsDir())
            {
              childDirs.push_back(childResource);
            }
            else
            {
              childFiles.push_back(childResource);
            }
          }
        }

        for (std::vector<us::ModuleResource>::iterator iter = childDirs.begin(),
             endIter = childDirs.end(); iter != endIter; ++iter)
        {
          if (iter != childDirs.begin())
          {
            json += ",\n";
          }
          GetJsonRepresentation(module, *iter, json, level+1);
        }
        for (std::vector<us::ModuleResource>::iterator iter = childFiles.begin(),
             endIter = childFiles.end(); iter != endIter; ++iter)
        {
          std::cout << "\t" << iter->GetResourcePath() << std::endl;
          if (iter != childFiles.begin() || !childDirs.empty())
          {
            json += ",\n";
          }
          GetJsonRepresentation(module, *iter, json, level+1);
        }
        json += "]}";
      }
    }
  }

  std::string getResourceUri(const std::string& uri)
  {
    std::string result;
    std::size_t pos = uri.find_last_of('/');
    if (pos == std::string::npos) return result;
    if (uri.substr(0, pos) != "/Console/modules" || pos == uri.size() - 1) return result;
    std::string id = uri.substr(pos+1);
    if (id.empty()) return result;

    result += "/Console/module.html";
    return result;
  }

};

US_END_NAMESPACE

#endif // USMODULESSERVLET_P_H
