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
#include "SCRExtensionRegistry.hpp"

namespace cppmicroservices
{
    namespace scrimpl
    {
        SCRExtensionRegistry::SCRExtensionRegistry(std::shared_ptr<cppmicroservices::logservice::LogService> logger)
            : logger(std::move(logger))
        {
            if (!(this->logger))
            {
                throw std::invalid_argument(" SCRExtensionRegistry Constructor "
                "provided with invalid arguments");
            }
        }

        std::shared_ptr<SCRBundleExtension>
        SCRExtensionRegistry::Find(long bundleId) noexcept
        {
            std::lock_guard<std::mutex> l(extensionRegMutex);
            if (auto const& it = extensionRegistry.find(bundleId); it != extensionRegistry.end())
            {
                return it->second;
            }
            return nullptr;
        }

        void
        SCRExtensionRegistry::Add(long bundleId, std::shared_ptr<SCRBundleExtension> extension)
        {
            if (!extension)
            {
                throw std::invalid_argument("SCRExtensionRegistry::Add invalid extension");	
            }
            std::lock_guard<std::mutex> l(extensionRegMutex);
            if (extensionRegistry.find(bundleId) == extensionRegistry.end())
            {
                extensionRegistry.insert(std::make_pair(bundleId, std::move(extension)));
            }
        }

        void
        SCRExtensionRegistry::Remove(long bundleId)
        {
            std::lock_guard<std::mutex> l(extensionRegMutex);
            extensionRegistry.erase(bundleId);
        }

        void
        SCRExtensionRegistry::Clear()
        {
            std::lock_guard<std::mutex> l(extensionRegMutex);
            extensionRegistry.clear();
         }
    } // namespace scrimpl
} // namespace cppmicroservices