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

#include "ComponentConfigurationFactory.hpp"
#include "SingletonComponentConfiguration.hpp"
#include "BundleOrPrototypeComponentConfiguration.hpp"

namespace cppmicroservices {
namespace scrimpl {

std::shared_ptr<ComponentConfigurationImpl> ComponentConfigurationFactory::CreateConfigurationManager(std::shared_ptr<const metadata::ComponentMetadata> compDesc,
                                                                                                      const cppmicroservices::Bundle& bundle,
                                                                                                      std::shared_ptr<const ComponentRegistry> registry,
                                                                                                      std::shared_ptr<logservice::LogService> logger)
{
  std::shared_ptr<ComponentConfigurationImpl> retVal;
  std::string scope = compDesc->serviceMetadata.scope;
  if(scope == cppmicroservices::Constants::SCOPE_SINGLETON)
  {
    retVal = std::make_shared<SingletonComponentConfigurationImpl>(compDesc,
                                                                   bundle,
                                                                   registry,
                                                                   logger);
  }
  else if (scope == cppmicroservices::Constants::SCOPE_BUNDLE ||
           scope == cppmicroservices::Constants::SCOPE_PROTOTYPE)
  {
    retVal = std::make_shared<BundleOrPrototypeComponentConfigurationImpl>(compDesc,
                                                                           bundle,
                                                                           registry,
                                                                           logger);
  }
  if(retVal)
  {
    retVal->Initialize();
  }
  return retVal;
}
}
}
