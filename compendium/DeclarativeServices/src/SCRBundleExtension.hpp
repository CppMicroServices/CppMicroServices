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

#ifndef SCRBUNDLEEXTENSION_HPP
#define SCRBUNDLEEXTENSION_HPP

#include <memory>
#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "ComponentRegistry.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/asyncworkservice/AsyncWorkService.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "manager/ComponentManager.hpp"
#include "manager/ConfigurationNotifier.hpp"
#include "metadata/Util.hpp"

using cppmicroservices::logservice::LogService;

namespace cppmicroservices
{
    namespace scrimpl
    {
        /**
         * The SCRBundleExtension is a helper class to load and unload Components of
         * a single bundle. It is responsible for creating a component manager for each
         * valid component description found in the bundle. On destruction, this object
         * removes and destroys the component managers created during it's construction
         */
        class SCRBundleExtension
        {
          public:
            /* SCRBundleExtension constructor
             * @param bundle { @link Bundle }
             * @param shared_ptr to {@link ComponentRegistry} registry 
             * @param shared_ptr to {@link LogService} logger
             * @param shared_ptr to {@link ConfigurationNotifier} object
             * @throws std::illegal_argument if any input parameters fail validation.
             */

            SCRBundleExtension(cppmicroservices::Bundle const& bundle,
                                                   std::shared_ptr<ComponentRegistry> const& registry,
                                                   std::shared_ptr<LogService> const& logger,
                                                   std::shared_ptr<ConfigurationNotifier> const& configNotifier);
   

            SCRBundleExtension(SCRBundleExtension const&) = delete;
            SCRBundleExtension(SCRBundleExtension&&) = delete;
            SCRBundleExtension& operator=(SCRBundleExtension const&) = delete;
            SCRBundleExtension& operator=(SCRBundleExtension&&) = delete;
            ~SCRBundleExtension();

             /*
             *@throws std::bad_alloc exception if a storage failure occurs on 
             * reallocation. Has the same exception guarantees as std::vector::push_back
             */
            void AddComponentManager(std::shared_ptr<ComponentManager>);

            /* SCRBundleExtension::Initialize - Creates the ComponentManagerImpl
             * object to manage the creation of the component configurations.
             * @param AnyMap containing the bundle metadata 
             * @param shared_ptr to {@link AsyncWorkService} object
             * @throws std::illegal_argument if any input parameters fail validation.
             * @throws {@link SharedLibraryException} if library cannot be loaded.
             * @throws {@link SecurityException} if bundle validation fails.
             * @throws std::exception for all other exceptions
             */

            void Initialize(cppmicroservices::AnyMap const& scrMetadata,
                            std::shared_ptr<cppmicroservices::async::AsyncWorkService> const& asyncWorkService);
 
          private:
            FRIEND_TEST(SCRBundleExtensionTest, CtorWithValidArgs);

            void DisableAndRemoveAllComponentManagers();

            cppmicroservices::Bundle bundle_;
            std::shared_ptr<ComponentRegistry> registry;
            std::shared_ptr<LogService> logger;
            std::vector<std::shared_ptr<ComponentManager>> managers;
            std::mutex managersMutex; // protects the managers
            std::shared_ptr<ConfigurationNotifier> configNotifier;
        };
    } // namespace scrimpl
} // namespace cppmicroservices

#endif // SCRBUNDLEEXTENSION_HPP
