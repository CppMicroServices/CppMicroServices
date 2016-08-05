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

#ifndef USRESOURCESSERVLET_H
#define USRESOURCESSERVLET_H

class ResourcesServlet
{
public:
  ResourcesServlet();

  bool handleGet(CivetServer* /*server*/, struct mg_connection *conn)
  {
    std::string uri = mg_get_request_info(conn)->uri;

    std::pair<long int, std::string> resourceData = getResourceData(uri);
    if (resourceData.first < 0 || resourceData.second.empty())
    {
      return false;
    }

    cppmicroservices::Bundle* bundle = cppmicroservices::BundleRegistry::GetBundle(resourceData.first);
    if (bundle == nullptr)
    {
      return false;
    }

    cppmicroservices::BundleResource resource = bundle->GetResource(resourceData.second);
    if (!resource)
    {
      MBI_INFO << "Resource " << resourceData.second << " not found";
      return false;
    }

    cppmicroservices::BundleResourceStream resourceStream(resource, std::ios::binary);
    std::string result;
    resourceStream.seekg(0, std::ios::end);
    result.resize(resourceStream.tellg());
    resourceStream.seekg(0, std::ios::beg);
    resourceStream.read(&result[0], result.size());

    const char* mimeType = mg_get_builtin_mime_type(resourceData.second.c_str());
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              /* "Cache: no-cache\r\n" */
              "Content-Type: %s\r\n"
              "Content-Length: %ld\r\n"
              "\r\n",
              mimeType, result.size());
    mg_write(conn, result.c_str(), result.size());
    return true;
  }

  std::pair<long int, std::string> getResourceData(const std::string& uri)
  {
    long int id = -1;
    std::string result;

    const std::string prefix = "/Console/resources/";
    if (uri.size() < prefix.size() + 1) return std::make_pair(-1, std::string(""));

    std::size_t pos = uri.find_first_of('/', prefix.size());
    if (pos == std::string::npos) return std::make_pair(-1, std::string(""));

    std::string strId = uri.substr(prefix.size(), pos - prefix.size());
    std::istringstream(strId) >> id;
    std::string resourceUri = uri.substr(pos);
    return std::make_pair(id, resourceUri);
  }
};

#endif // USRESOURCESSERVLET_H
