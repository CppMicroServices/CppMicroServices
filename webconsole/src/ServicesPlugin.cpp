/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "ServicesPlugin.h"

#include "cppmicroservices/webconsole/WebConsoleDefaultVariableResolver.h"

#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/GetBundleContext.h"

#include <set>

namespace cppmicroservices {

std::string NumToString(int64_t val);

ServicesPlugin::ServicesPlugin()
  : SimpleWebConsolePlugin("services", "Services", "")
{}

void ServicesPlugin::RenderContent(HttpServletRequest& request,
                                   HttpServletResponse& response)
{
  std::string pathInfo = request.GetPathInfo();
  if (pathInfo == "/services") {
    BundleResource res =
      GetBundleContext().GetBundle().GetResource("/templates/services.html");
    if (res) {
      auto& data = std::static_pointer_cast<WebConsoleDefaultVariableResolver>(
                     GetVariableResolver(request))
                     ->GetData();
      data["services"] = GetIds();

      BundleResourceStream rs(res, std::ios_base::binary);
      response.GetOutputStream() << rs.rdbuf();
    }
  } else if (pathInfo.size() > 20 &&
             pathInfo.compare(0, 20, "/services/interface/") == 0) {
    std::string id = pathInfo.substr(20);
    BundleResource res = GetBundleContext().GetBundle().GetResource(
      "/templates/service_interface.html");
    if (res) {
      auto& data = std::static_pointer_cast<WebConsoleDefaultVariableResolver>(
                     GetVariableResolver(request))
                     ->GetData();
      data["interface"] = id;
      data["services"] = GetInterface(id);

      BundleResourceStream rs(res, std::ios_base::binary);
      response.GetOutputStream() << rs.rdbuf();
    }
  }
}

AbstractWebConsolePlugin::TemplateData ServicesPlugin::GetIds() const
{
  std::set<std::string> ids;
  std::vector<ServiceReferenceU> refs = GetContext().GetServiceReferences("");
  for (auto ref : refs) {
    Any objectClass = ref.GetProperty(Constants::OBJECTCLASS);
    auto const& oc = ref_any_cast<std::vector<std::string>>(objectClass);
    for (auto const& id : oc) {
      ids.insert(id);
    }
  }

  TemplateData data(TemplateData::Type::List);
  for (auto const& id : ids) {
    data << TemplateData{ "id", id };
  }
  return data;
}

AbstractWebConsolePlugin::TemplateData ServicesPlugin::GetInterface(
  const std::string& iid) const
{
  TemplateData data(TemplateData::Type::List);

  for (auto& ref : GetContext().GetServiceReferences(iid)) {
    AnyMap props(AnyMap::ORDERED_MAP);
    for (auto const& key : ref.GetPropertyKeys()) {
      props[key] = ref.GetProperty(key);
    }

    TemplateData entry;
    entry["bundle"] = ref.GetBundle().GetSymbolicName();
    entry["bundle-id"] = NumToString(ref.GetBundle().GetBundleId());
    entry["id"] = ref.GetProperty(Constants::SERVICE_ID).ToStringNoExcept();
    entry["ranking"] =
      ref.GetProperty(Constants::SERVICE_RANKING).ToStringNoExcept();
    entry["scope"] =
      ref.GetProperty(Constants::SERVICE_SCOPE).ToStringNoExcept();
    entry["types"] = ref.GetProperty(Constants::OBJECTCLASS).ToStringNoExcept();
    entry["props"] = Any(props).ToJSON();

    data << std::move(entry);
  }

  return data;
}
}
