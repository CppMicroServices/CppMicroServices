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

#ifndef CPPMICROSERVICES_SCRIMPL_COMPONENTFACTORYIMPL_HPP__
#define CPPMICROSERVICES_SCRIMPL_COMPONENTFACTORYIMPL_HPP__

#include "../SCRLogger.hpp"
#include "cppmicroservices/asyncworkservice/AsyncWorkService.hpp"
#include "cppmicroservices/cm/ConfigurationListener.hpp"

namespace cppmicroservices
{
namespace scrimpl {
    class SCRExtensionRegistry;
    class ComponentConfigurationImpl;
    class ComponentFactoryImpl final
    {

      public:
        /**
         * @throws std::invalid_argument exception if any of the params is a nullptr
         */
        ComponentFactoryImpl(cppmicroservices::BundleContext const& context,
                             std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                             std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkSvc,
                             std::shared_ptr<SCRExtensionRegistry> extensionReg);

        ComponentFactoryImpl(ComponentFactoryImpl const&) = delete;
        ComponentFactoryImpl(ComponentFactoryImpl&&) = delete;
        ComponentFactoryImpl& operator=(ComponentFactoryImpl const&) = delete;
        ComponentFactoryImpl& operator=(ComponentFactoryImpl&&) = delete;
        ~ComponentFactoryImpl() = default;

        /**
         * @brief Creates a factory instance
         * @param pid The factory pid. Format factory~instance
         * @param mgr The ComponentConfigurationImpl object belonging to the factory component
         * @param properties The properties from the Config Admin configuration object belonging to the pid.
         * @throws std::invalid_argument exception if a dynamic target contains an invalid LDAPFilter.
         * @throws std::exception if the factory instance is not successfully created.
         */
        void CreateFactoryComponent(std::string const& pid,
                                        std::shared_ptr<ComponentConfigurationImpl>& mgr,
                                        cppmicroservices::AnyMap const& properties);

      private:
         cppmicroservices::BundleContext bundleContext;
         std::shared_ptr<cppmicroservices::logservice::LogService> logger;
         std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkService;
         std::shared_ptr<SCRExtensionRegistry> extensionRegistry;
    };

} // scrimpl namespace
} // cppmicroservices namespace
#endif //__CPPMICROSERVICES_SCRIMPL_COMPONENTFACTORYIMPL_HPP__
