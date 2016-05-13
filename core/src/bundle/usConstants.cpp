/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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


#include "usConstants.h"

#include <string>

namespace us US_Core_EXPORT {

namespace Constants {

#if !defined(__clang__)
#undef US_Core_EXPORT
#define US_Core_EXPORT
#endif

US_Core_EXPORT const std::string SYSTEM_BUNDLE_LOCATION              = "System Bundle";
US_Core_EXPORT const std::string SYSTEM_BUNDLE_SYMBOLICNAME          = "system_bundle";

US_Core_EXPORT const std::string BUNDLE_CATEGORY                     = "bundle.category";
US_Core_EXPORT const std::string BUNDLE_COPYRIGHT                    = "bundle.copyright";
US_Core_EXPORT const std::string BUNDLE_DESCRIPTION                  = "bundle.description";
US_Core_EXPORT const std::string BUNDLE_NAME                         = "bundle.name";
US_Core_EXPORT const std::string BUNDLE_VENDOR                       = "bundle.vendor";
US_Core_EXPORT const std::string BUNDLE_VERSION                      = "bundle.version";
US_Core_EXPORT const std::string BUNDLE_DOCURL                       = "bundle.doc_url";
US_Core_EXPORT const std::string BUNDLE_CONTACTADDRESS               = "bundle.contact_address";
US_Core_EXPORT const std::string BUNDLE_SYMBOLICNAME                 = "bundle.symbolic_name";
US_Core_EXPORT const std::string BUNDLE_MANIFESTVERSION              = "bundle.manifest_version";
US_Core_EXPORT const std::string BUNDLE_ACTIVATIONPOLICY             = "bundle.activation_policy";
US_Core_EXPORT const std::string ACTIVATION_LAZY                     = "lazy";
US_Core_EXPORT const std::string FRAMEWORK_VERSION                   = "org.cppmicroservices.framework.version";
US_Core_EXPORT const std::string FRAMEWORK_VENDOR                    = "org.cppmicroservices.framework.vendor";
US_Core_EXPORT US_Core_EXPORT const std::string FRAMEWORK_STORAGE                   = "org.cppmicroservices.framework.storage";
US_Core_EXPORT const std::string FRAMEWORK_STORAGE_CLEAN             = "org.cppmicroservices.framework.storage.clean";
US_Core_EXPORT const std::string FRAMEWORK_STORAGE_CLEAN_ONFIRSTINIT = "onFirstInit";
US_Core_EXPORT const std::string FRAMEWORK_THREADING_SUPPORT         = "org.cppmicroservices.framework.threading.support";
US_Core_EXPORT const std::string FRAMEWORK_LOG_LEVEL                 = "org.cppmicroservices.framework.log.level";
US_Core_EXPORT const std::string FRAMEWORK_UUID                      = "org.cppmicroservices.framework.uuid";

US_Core_EXPORT const std::string OBJECTCLASS                         = "objectclass";
US_Core_EXPORT const std::string SERVICE_ID                          = "service.id";
US_Core_EXPORT const std::string SERVICE_PID                         = "service.pid";
US_Core_EXPORT const std::string SERVICE_RANKING                     = "service.ranking";
US_Core_EXPORT const std::string SERVICE_VENDOR                      = "service.vendor";
US_Core_EXPORT const std::string SERVICE_DESCRIPTION                 = "service.description";
US_Core_EXPORT const std::string SERVICE_SCOPE                       = "service.scope";
US_Core_EXPORT const std::string SCOPE_SINGLETON                     = "singleton";
US_Core_EXPORT const std::string SCOPE_BUNDLE                        = "bundle";
US_Core_EXPORT const std::string SCOPE_PROTOTYPE                     = "prototype";

}

}
