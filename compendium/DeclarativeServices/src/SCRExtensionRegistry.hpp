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

#ifndef __CPPMICROSERVICES_SCRIMPL_SCREXTENSIONREGISTRY_HPP__
#define __CPPMICROSERVICES_SCRIMPL_SCREXTENSIONREGISTRY_HPP__

#include "SCRBundleExtension.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

#include <map>
#include <memory>
#include <mutex>

namespace cppmicroservices
{
    namespace scrimpl
    {
        /* The SCRBundleExtension is a helper class to load and unload Components of
         * a single bundle. It is responsible for creating a component manager for each
         * valid component description found in the bundle. The SCRExtensionRegistry 
         * maintains a map of SCRBundleExtension objects that is accesed by BundleId.
         */
        class SCRExtensionRegistry 
        {

          public:
            /**
             * @throws std::invalid_argument exception if any of the params is a nullptr
             */
            SCRExtensionRegistry(std::shared_ptr<cppmicroservices::logservice::LogService> logger);
 
            SCRExtensionRegistry(SCRExtensionRegistry const&) = delete;
            SCRExtensionRegistry(SCRExtensionRegistry&&) = delete;
            SCRExtensionRegistry& operator=(SCRExtensionRegistry const&) = delete;
            SCRExtensionRegistry& operator=(SCRExtensionRegistry&&) = delete;
            ~SCRExtensionRegistry() = default;

            /* SCRExtensionRegistry::Find 
             * Searches the extensionRegistry for the bundleId matching the input 
             * parameter. 
             * @param bundleId The bundleId {@link Bundle} associated with the 
             * {@link SCRBundleExtension} responsible for creating the components.
             * @returns shared_ptr to a {@link SCRBundleExtension} object or nullptr 
             * if the no matching bundleId is found. 
             */
            std::shared_ptr<SCRBundleExtension> Find(long bundleId) noexcept;
           
            /* SCRExtensionRegistry::Add
             * Inserts a SCRBundleExtension object into the extensionRegistry using
             * the bundleId as the key.
             * @param bundleId The bundleId {@link Bundle} associated with the
             * {@link SCRBundleExtension} object to be inserted.
             * @param shared_ptr to the {@link SCRBundleExtension} object to be inserted. 
             * @throws std::invalid_argument if the extension input parameter if invalid.
             * @throws std::bad_alloc exception if a storage failure occurs. 
             */
            void Add(long bundleId, std::shared_ptr<SCRBundleExtension> extension);

            /* SCRExtensionRegistry::Remove
             * Removes a SCRBundleExtension object from the extensionRegistry using
             * the bundleId as the key.
             * @param bundleId The bundleId {@link Bundle} associated with the
             * {@link SCRBundleExtension} object to be removed.
             * @throws std::current_exception if an exception is thrown by the 
             * {@link SCRBundleExtension} destructor
             */
            void Remove(long bundleId);

            /* SCRExtensionRegistry::Clears
             * Removes all  SCRBundleExtension objects from the extensionRegistry.
             * @throws std::current_exception if an exception is thrown by a
             * {@link SCRBundleExtension} destructor
             */
            void Clear();

          private:
            std::shared_ptr<cppmicroservices::logservice::LogService> logger;
            std::mutex extensionRegMutex;  //protects the extensionRegistry
            std::unordered_map<long,std::shared_ptr<SCRBundleExtension>> extensionRegistry;
        };

    } // namespace scrimpl
} // namespace cppmicroservices
#endif //__CPPMICROSERVICES_SCRIMPL_SCREXTENSIONREGISTRY_HPP__
