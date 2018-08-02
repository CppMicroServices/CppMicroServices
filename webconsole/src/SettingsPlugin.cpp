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

#include "SettingsPlugin.h"

#include "cppmicroservices/webconsole/WebConsoleDefaultVariableResolver.h"

#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleResource.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/GetBundleContext.h"

namespace cppmicroservices {

SettingsPlugin::SettingsPlugin()
  : SimpleWebConsolePlugin("settings", "Settings", "")
{}

void SettingsPlugin::RenderContent(HttpServletRequest& request,
                                   HttpServletResponse& response)
{
  BundleResource res =
    GetBundleContext().GetBundle().GetResource("/templates/settings.html");
  if (res) {
    auto props = GetBundleContext().GetProperties();
    auto& data = std::static_pointer_cast<WebConsoleDefaultVariableResolver>(
                   GetVariableResolver(request))
                   ->GetData();
    data["us-thread"] =
      props[Constants::FRAMEWORK_THREADING_SUPPORT].ToStringNoExcept() ==
          Constants::FRAMEWORK_THREADING_MULTI
        ? TemplateData::Type::True
        : TemplateData::Type::False;
#ifdef US_BUILD_SHARED_LIBS
    data["us-shared"] = TemplateData::Type::True;
#else
    data["us-shared"] = TemplateData::Type::False;
#endif
    data["us-storagepath"] =
      props[Constants::FRAMEWORK_STORAGE].ToStringNoExcept();

    TemplateData fwProps(TemplateData::Type::List);
    for (auto p : props) {
      TemplateData kv;
      kv["key"] = p.first;
      kv["value"] = p.second.ToString();
      fwProps << kv;
    }
    data["us-fwprops"] = std::move(fwProps);

    BundleResourceStream rs(res, std::ios_base::binary);
    response.GetOutputStream() << rs.rdbuf();
  }
}

BundleResource SettingsPlugin::GetResource(const std::string& path) const
{
  return (this->GetContext()) ? this->GetContext().GetBundle().GetResource(path)
                              : BundleResource();
}
}
