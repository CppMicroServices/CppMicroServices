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

#include "SingletonComponentConfiguration.hpp"
#include "../ComponentRegistry.hpp"
#include "ComponentManager.hpp"
#include "ReferenceManager.hpp"
#include "ReferenceManagerImpl.hpp"
#include "RegistrationManager.hpp"
#include "SingleInvokeTask.hpp"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "states/CCUnsatisfiedReferenceState.hpp"
#include "states/ComponentConfigurationState.hpp"
#include <iostream>

using cppmicroservices::scrimpl::ReferenceManagerImpl;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_ID;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME;

namespace cppmicroservices
{
    namespace scrimpl
    {

        SingletonComponentConfigurationImpl::SingletonComponentConfigurationImpl(
            std::shared_ptr<metadata::ComponentMetadata const> metadata,
            Bundle const& bundle,
            std::shared_ptr<ComponentRegistry> registry,
            std::shared_ptr<cppmicroservices::logservice::LogService> logger,
            std::shared_ptr<ConfigurationNotifier> configNotifier)
            : ComponentConfigurationImpl(metadata, bundle, registry, logger, configNotifier)
        {
        }

        std::shared_ptr<ServiceFactory>
        SingletonComponentConfigurationImpl::GetFactory()
        {
            std::shared_ptr<SingletonComponentConfigurationImpl> thisPtr
                = std::dynamic_pointer_cast<SingletonComponentConfigurationImpl>(shared_from_this());
            return ToFactory(thisPtr);
        }

        SingletonComponentConfigurationImpl::~SingletonComponentConfigurationImpl() { DestroyComponentInstances(); }

        std::shared_ptr<ComponentInstance>
        SingletonComponentConfigurationImpl::CreateAndActivateComponentInstance(
            cppmicroservices::Bundle const& /*bundle*/)
        {
            auto instanceContextPair = data.lock();
            if (GetState()->GetValue() != service::component::runtime::dto::ComponentState::ACTIVE)
            {
                GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                                 "Activate failed. Component no longer in Active State.");
                return nullptr;
            }

            if (!instanceContextPair->first)
            {
                try
                {
                    auto instCtxtTuple = CreateAndActivateComponentInstanceHelper(Bundle());
                    instanceContextPair->first = instCtxtTuple.first;
                    instanceContextPair->second = instCtxtTuple.second;
                }
                catch (cppmicroservices::SharedLibraryException const&)
                {
                    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                     "Exception thrown while trying to load a shared library",
                                     std::current_exception());
                    throw;
                }
                catch (cppmicroservices::SecurityException const&)
                {
                    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                     "Exception thrown while trying to validate a bundle",
                                     std::current_exception());
                    throw;
                }
                catch (...)
                {
                    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                     "Exception received from user code while activating the "
                                     "component configuration",
                                     std::current_exception());
                }
            }
            return instanceContextPair->first;
        }
        bool
        SingletonComponentConfigurationImpl::ModifyComponentInstanceProperties()
        {
            auto instanceContextPair = data.lock();
            if (instanceContextPair->first)
            {
                try
                {
                    if (instanceContextPair->first->DoesModifiedMethodExist())
                    {
                        instanceContextPair->first->Modified();
                        return true;
                    }
                }
                catch (...)
                {
                    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                     "Exception received from user code while modifying "
                                     "component configuration",
                                     std::current_exception());
                    return false;
                }
            }
            return false;
        }

        void
        SingletonComponentConfigurationImpl::DestroyComponentInstances()
        {
            auto instanceContextPair = data.lock();
            try
            {
                if (instanceContextPair->first)
                {
                    instanceContextPair->first->Deactivate();
                    instanceContextPair->first->UnbindReferences();
                }
            }
            catch (...)
            {
                GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                 "Exception received from user code while deactivating the "
                                 "component configuration",
                                 std::current_exception());
            }
            if (instanceContextPair->second)
            {
                instanceContextPair->second->Invalidate();
            }
            instanceContextPair->first.reset();
            instanceContextPair->second.reset();
        }

        InterfaceMapConstPtr
        SingletonComponentConfigurationImpl::GetService(cppmicroservices::Bundle const& bundle,
                                                        cppmicroservices::ServiceRegistrationBase const& registration)
        {
            // if activation passed, return the interface map from the instance
            std::shared_ptr<cppmicroservices::service::component::detail::ComponentInstance> compInstance;
            try
            {
                compInstance = Activate(bundle);
            }
            catch (cppmicroservices::SecurityException const&)
            {
                auto compManagerRegistry = GetRegistry();
                auto compMgrs
                    = compManagerRegistry->GetComponentManagers(registration.GetReference().GetBundle().GetBundleId());
                std::for_each(compMgrs.begin(),
                              compMgrs.end(),
                              [this](std::shared_ptr<cppmicroservices::scrimpl::ComponentManager> const& compMgr)
                              {
                                  try
                                  {
                                      auto singleInvoke = std::make_shared<SingleInvokeTask>();
                                      auto f = compMgr->Disable(singleInvoke);
                                      compMgr->WaitForFuture(f, singleInvoke);
                                  }
                                  catch (...)
                                  {
                                      std::string errMsg("A security exception handler caused a component manager "
                                                         "to disable, leading to an exception disabling "
                                                         "component manager: ");
                                      errMsg += compMgr->GetName();
                                      GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                                                       errMsg,
                                                       std::current_exception());
                                  }
                              });
                throw;
            }
            return compInstance ? compInstance->GetInterfaceMap() : nullptr;
        }

        void
        SingletonComponentConfigurationImpl::UngetService(
            cppmicroservices::Bundle const& /*bundle*/,
            cppmicroservices::ServiceRegistrationBase const& /*registration*/,
            cppmicroservices::InterfaceMapConstPtr const& /*service*/)
        {
            // The singleton instance is not reset when UngetService is called.
            // The instance is reset when the component is deactivated.
        }

        void
        SingletonComponentConfigurationImpl::BindReference(std::string const& refName, ServiceReferenceBase const& ref)
        {
            auto context = GetComponentContext();
            if (!context)
            {
                throw std::runtime_error("Invalid Singleton Component Context, likely destroyed");
            }
            auto svcToBind = context->AddToBoundServicesCache(refName, ref);
            if (!svcToBind)
            {
                GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                                 "Failure while adding reference " + refName + " to the bound services cache.");
                return;
            }
            try
            {
                if (auto inst = GetComponentInstance(); inst)
                {
                    inst->InvokeBindMethod(refName, svcToBind);
                }
                else
                {
                    throw std::runtime_error("Invalid Singleton Component Instance, likely destroyed");
                }
            }
            catch (std::exception const& ex)
            {
                GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                 "Exception received from user code while binding "
                                 "service reference"
                                     + refName + ".",
                                 std::current_exception());
                std::cout << ex.what() << std::endl;
            }
        }

        void
        SingletonComponentConfigurationImpl::UnbindReference(std::string const& refName,
                                                             ServiceReferenceBase const& ref)
        {
            auto context = GetComponentContext();
            std::cout << any_cast<int>(ref.GetProperty(Constants::SERVICE_RANKING)) << std::endl;
            if (!context)
            {
                throw std::runtime_error("Invalid Singleton Component Context, likely destroyed");
            }
            try
            {
                auto svcToUnbind = context->LocateService(refName, ref);
                if (auto inst = GetComponentInstance(); inst)
                {
                    inst->InvokeUnbindMethod(refName, svcToUnbind);
                }
                else
                {
                    throw std::runtime_error("Invalid Singleton Component Instance, likely destroyed");
                }
            }
            catch (std::exception const& ex)
            {
                GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                                 "Exception received from user code while unbinding "
                                 "service reference"
                                     + refName + ".",
                                 std::current_exception());
                std::cout << ex.what() << std::endl;
            }
            context->RemoveFromBoundServicesCache(refName, ref);
        }

        void
        SingletonComponentConfigurationImpl::SetComponentInstancePair(InstanceContextPair instCtxtPair)
        {
            auto instanceContextPair = data.lock();
            instanceContextPair->first = instCtxtPair.first;
            instanceContextPair->second = instCtxtPair.second;
        }

        std::shared_ptr<ComponentContextImpl>
        SingletonComponentConfigurationImpl::GetComponentContext()
        {
            return data.lock()->second;
        }

        std::shared_ptr<ComponentInstance>
        SingletonComponentConfigurationImpl::GetComponentInstance()
        {
            return data.lock()->first;
        }
    } // namespace scrimpl
} // namespace cppmicroservices
