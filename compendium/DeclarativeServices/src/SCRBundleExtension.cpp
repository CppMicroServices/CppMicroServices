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

#include "SCRBundleExtension.hpp"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "manager/ComponentManagerImpl.hpp"
#include "metadata/ComponentMetadata.hpp"
#include "metadata/MetadataParser.hpp"
#include "metadata/MetadataParserFactory.hpp"
#include "metadata/Util.hpp"

using cppmicroservices::service::component::ComponentConstants::SERVICE_COMPONENT;

namespace cppmicroservices {
namespace scrimpl {

using metadata::ComponentMetadata;
using util::ObjectValidator;
SCRBundleExtension::SCRBundleExtension(const cppmicroservices::BundleContext& bundleContext,
                                       const cppmicroservices::AnyMap& scrMetadata,
                                       const std::shared_ptr<ComponentRegistry>& registry,
                                       const std::shared_ptr<LogService>& logger)
  : bundleContext(bundleContext)
  , registry(registry)
  , logger(logger)
{
  if(!bundleContext || !registry || !logger || scrMetadata.empty())
  {
    throw std::invalid_argument("Invalid parameters passed to SCRBundleExtension constructor");
  }

  auto version = ObjectValidator(scrMetadata, "version").GetValue<int>();
  auto metadataparser = metadata::MetadataParserFactory::Create(version, logger);
  std::vector<std::shared_ptr<ComponentMetadata>> componentsMetadata;
  componentsMetadata = metadataparser->ParseAndGetComponentsMetadata(scrMetadata);
  for (auto& oneCompMetadata : componentsMetadata)
  {
    try
    {
      auto compManager = std::make_shared<ComponentManagerImpl>(oneCompMetadata,
                                                                registry,
                                                                bundleContext,
                                                                logger);
      if(registry->AddComponentManager(compManager))
      {
        managers.push_back(compManager);
        compManager->Initialize();
      }
    } catch (const cppmicroservices::SharedLibraryException&) {
      throw;
    } catch (const std::exception&) {
      logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                  "Failed to create ComponentManager with name " + oneCompMetadata->name + " from bundle with Id " + std::to_string(bundleContext.GetBundle().GetBundleId()),
                  std::current_exception());
    }
  }
  logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
              "Created instance of SCRBundleExtension for " + bundleContext.GetBundle().GetSymbolicName());
}

SCRBundleExtension::~SCRBundleExtension()
{
  logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG,
              "Deleting instance of SCRBundleExtension for " + bundleContext.GetBundle().GetSymbolicName());
  for(auto compManager : managers)
  {
    auto fut = compManager->Disable();
    registry->RemoveComponentManager(compManager);
    fut.get(); // since this happens when the bundle is stopped. Wait until the disable is finished on the other thread.
  }
  managers.clear();
  registry.reset();
};
} // scrimpl
} // cppmicroservices

