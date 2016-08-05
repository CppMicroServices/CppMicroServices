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

#ifndef USBUNDLESSERVLET_P_H
#define USBUNDLESSERVLET_P_H

#include "SimpleWebConsolePlugin.h"

namespace cppmicroservices {

class BundlesServlet : public SimpleWebConsolePlugin
{
  BundlesHtml(cppmicroservices::BundleContext* context)
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

    cppmicroservices::BundleResource resource = m_Context->GetBundle()->GetResource(resourceUri);
    if (!resource)
    {
      MBI_INFO << "Resource " << resourceUri << " not found";
      return false;
    }

    cppmicroservices::BundleResourceStream resourceStream(resource, std::ios::binary);
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
      if (uri == "/Console/bundles")
      {
        json += "{ \"bundles\": [";
        std::vector<cppmicroservices::Bundle*> bundles = m_Context->GetBundles();
        for(std::vector<cppmicroservices::Bundle*>::iterator iter = bundles.begin(), endIter = bundles.end();
            iter != endIter; ++iter)
        {
          if (iter != bundles.begin())
          {
            json += ",";
          }
          json += "{\n  \"id\": " + cpptempl::lexical_cast((*iter)->GetBundleId()) + ",\n";
          json += "  \"name\": \"" + (*iter)->GetName() + "\",\n";
          cppmicroservices::Any description = (*iter)->GetProperty(cppmicroservices::Bundle::PROP_DESCRIPTION());
          json += "  \"description\": \"" + (description.Empty() ? std::string() : description.ToString()) + "\",\n";
          json += "  \"version\": \"" + (*iter)->GetVersion().ToString() + "\",\n";
          cppmicroservices::Any vendor = (*iter)->GetProperty(cppmicroservices::Bundle::PROP_VENDOR());
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
    if (uri.substr(0, pos) != "/Console/bundles" || pos == uri.size() - 1) return false;
    std::string strId = uri.substr(pos+1);
    if (strId.empty()) return false;
    long int id = -1;
    std::istringstream(strId) >> id;
    if (id < 0) return false;

    cppmicroservices::Bundle* bundle = cppmicroservices::BundleRegistry::GetBundle(id);
    if (bundle == nullptr) return false;

    // ----------------- Get bundle properties ---------------------

    std::string json = "{ \"id\": " + strId + ",\n\"name\":\"" + bundle->GetName() + "\",\n\"properties\": [\n";
    std::vector<std::string> keys = bundle->GetPropertyKeys();
    bool separatorNeeded = false;
    for (std::vector<std::string>::iterator iter = keys.begin(),
         endIter = keys.end(); iter != endIter; ++iter)
    {
      cppmicroservices::Any value = bundle->GetProperty(*iter);
      if (!value.Empty())
      {
        if (separatorNeeded) json += ",";
        else separatorNeeded = true;
        json += "{ \"key\":\"" + *iter + "\", \"value\":\"" + value.ToString() + "\"}\n";
      }
    }
    json += "]";

    // --------------- Get bundle resource information ------------------

    cppmicroservices::BundleResource resource = bundle->GetResource("/");
    if (resource.IsValid())
    {
      json += ",\n\"resources\":\n";
      GetJsonRepresentation(bundle, resource, json, 0);
    }

    // ------------- Get registered service information ----------------
    std::vector<cppmicroservices::ServiceReferenceU> registeredServices = bundle->GetRegisteredServices();
    if (!registeredServices.empty())
    {
      json += ",\n\"registered_services\": [";
      for (std::vector<cppmicroservices::ServiceReferenceU>::iterator iter = registeredServices.begin(),
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

  void GetJsonRepresentation(cppmicroservices::Bundle* bundle, const cppmicroservices::BundleResource& currResource, std::string& json, int level)
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
              indent + "  \"href\":\"/Console/resources/" + cpptempl::lexical_cast(bundle->GetBundleId()) + currResource.GetResourcePath() + "\"\n" +
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
        std::vector<cppmicroservices::BundleResource> childDirs;
        std::vector<cppmicroservices::BundleResource> childFiles;
        for (std::vector<std::string>::iterator iter = children.begin(),
             endIter = children.end(); iter != endIter; ++iter)
        {
          cppmicroservices::BundleResource childResource = bundle->GetResource(currResource.GetResourcePath() + "/" + *iter);
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

        for (std::vector<cppmicroservices::BundleResource>::iterator iter = childDirs.begin(),
             endIter = childDirs.end(); iter != endIter; ++iter)
        {
          if (iter != childDirs.begin())
          {
            json += ",\n";
          }
          GetJsonRepresentation(bundle, *iter, json, level+1);
        }
        for (std::vector<cppmicroservices::BundleResource>::iterator iter = childFiles.begin(),
             endIter = childFiles.end(); iter != endIter; ++iter)
        {
          std::cout << "\t" << iter->GetResourcePath() << std::endl;
          if (iter != childFiles.begin() || !childDirs.empty())
          {
            json += ",\n";
          }
          GetJsonRepresentation(bundle, *iter, json, level+1);
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
    if (uri.substr(0, pos) != "/Console/bundles" || pos == uri.size() - 1) return result;
    std::string id = uri.substr(pos+1);
    if (id.empty()) return result;

    result += "/Console/bundle.html";
    return result;
  }

};

}

#endif // USBUNDLESSERVLET_P_H
