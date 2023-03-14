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

#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/SharedLibraryException.h"

#include "../ComponentRegistry.hpp"
#include "BundleOrPrototypeComponentConfiguration.hpp"
#include "ComponentManager.hpp"

namespace cppmicroservices {
namespace scrimpl {

BundleOrPrototypeComponentConfigurationImpl::
  BundleOrPrototypeComponentConfigurationImpl(
    std::shared_ptr<const metadata::ComponentMetadata> metadata,
    const cppmicroservices::Bundle& bundle,
    std::shared_ptr<ComponentRegistry> registry,
    std::shared_ptr<cppmicroservices::logservice::LogService> logger,
    std::shared_ptr<ConfigurationNotifier> configNotifier,
    std::shared_ptr<std::vector<std::shared_ptr<ComponentManager>>> managers)
  : ComponentConfigurationImpl(metadata,
                               bundle,
                               registry,
                               logger,
                               configNotifier,
                               managers)
{
}

std::shared_ptr<ServiceFactory>
BundleOrPrototypeComponentConfigurationImpl::GetFactory()
{
  auto thisPtr =
    std::dynamic_pointer_cast<BundleOrPrototypeComponentConfigurationImpl>(
      shared_from_this());
  return ToFactory(thisPtr);
}

BundleOrPrototypeComponentConfigurationImpl::
  ~BundleOrPrototypeComponentConfigurationImpl()
{
  DestroyComponentInstances();
}

std::shared_ptr<ComponentInstance>
BundleOrPrototypeComponentConfigurationImpl::CreateAndActivateComponentInstance(
  const cppmicroservices::Bundle& bundle)
{
  if (GetState()->GetValue() !=
      service::component::runtime::dto::ComponentState::ACTIVE) {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
                     "Activate failed. Component no longer in Active State.");
    return nullptr;
  }
  auto compInstCtxtPairList = compInstanceMap.lock();
  try {
    auto instCtxtTuple = CreateAndActivateComponentInstanceHelper(bundle);
    compInstCtxtPairList->emplace_back(instCtxtTuple);
    return instCtxtTuple.first;
  } catch (const cppmicroservices::SharedLibraryException&) {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                     "Exception thrown while trying to load a shared library",
                     std::current_exception());
    throw;
  } catch (const cppmicroservices::SecurityException&) {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                     "Exception thrown while trying to validate a bundle",
                     std::current_exception());
    throw;
  } catch (...) {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                     "Exception received from user code while activating the "
                     "component configuration",
                     std::current_exception());
  }
  return nullptr;
}

bool BundleOrPrototypeComponentConfigurationImpl::
  ModifyComponentInstanceProperties()
{
  auto compInstCtxtPairList = compInstanceMap.lock();
  bool retValue = false;
  for (const auto& valPair : *compInstCtxtPairList) {
    try {
      if (valPair.first->DoesModifiedMethodExist()) {
        valPair.first->Modified();
        retValue = true;
      }
    } catch (...) {
      GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                       "Exception received from user code while modifying "
                       "component configuration",
                       std::current_exception());
      return false;
    }
  }
  // ModifyComponentInstanceProperties returns true if the component instance has a Modified method.
  // Only need to return the value for the last instance because if one of the instances
  // has a Modified method, they all do.
  return retValue;
}

void BundleOrPrototypeComponentConfigurationImpl::DestroyComponentInstances()
{
  auto compInstCtxtPairList = compInstanceMap.lock();
  for (auto& valPair : *compInstCtxtPairList) {
    DeactivateComponentInstance(valPair);
  }
  compInstCtxtPairList->clear();
}

void BundleOrPrototypeComponentConfigurationImpl::DeactivateComponentInstance(
  const InstanceContextPair& instCtxt)
{
  try {
    instCtxt.first->Deactivate();
    instCtxt.first->UnbindReferences();
  } catch (...) {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                     "Exception received from user code while deactivating the "
                     "component configuration",
                     std::current_exception());
  }
  instCtxt.second->Invalidate();
}

InterfaceMapConstPtr BundleOrPrototypeComponentConfigurationImpl::GetService(
  const cppmicroservices::Bundle& bundle,
  const cppmicroservices::ServiceRegistrationBase& registration)
{
  // if activation passed, return the interface map from the instance
  std::shared_ptr<
    cppmicroservices::service::component::detail::ComponentInstance>
    compInstance;
  try {
    compInstance = Activate(bundle);
  } catch (const cppmicroservices::SecurityException&) {
    auto compManagerRegistry = GetRegistry();
    auto compMgrs = compManagerRegistry->GetComponentManagers(
      registration.GetReference().GetBundle().GetBundleId());
    std::for_each(
      compMgrs.begin(),
      compMgrs.end(),
      [this](const std::shared_ptr<cppmicroservices::scrimpl::ComponentManager>&
               compMgr) {
        try {
          compMgr->Disable().get();
        } catch (...) {
          std::string errMsg(
            "A security exception handler caused a component manager "
            "to disable, leading to an exception disabling "
            "component manager: ");
          errMsg += compMgr->GetName();
          GetLogger()->Log(
            cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
            errMsg,
            std::current_exception());
        }
      });
    throw;
  }
  return compInstance ? compInstance->GetInterfaceMap() : nullptr;
}

void BundleOrPrototypeComponentConfigurationImpl::UngetService(
  const cppmicroservices::Bundle&,
  const cppmicroservices::ServiceRegistrationBase& /*registration*/,
  const cppmicroservices::InterfaceMapConstPtr& service)
{
  std::shared_ptr<void> obj = cppmicroservices::ExtractInterface(service, "");
  auto compInstCtxtPairList = compInstanceMap.lock();
  auto itr =
    std::find_if(compInstCtxtPairList->begin(),
                 compInstCtxtPairList->end(),
                 [&obj](const InstanceContextPair& instCtxtPair) {
                   return (obj == cppmicroservices::ExtractInterface(
                                    instCtxtPair.first->GetInterfaceMap(), ""));
                 });
  if (itr != compInstCtxtPairList->end()) {
    DeactivateComponentInstance(*itr);
    compInstCtxtPairList->erase(itr);
  }
}

void BundleOrPrototypeComponentConfigurationImpl::BindReference(
  const std::string& refName,
  const ServiceReferenceBase& ref)
{
  auto instancePairs = compInstanceMap.lock();
  for (auto const& instancePair : *instancePairs) {
    auto& instance = instancePair.first;
    auto& context = instancePair.second;
    if (!context->AddToBoundServicesCache(refName, ref)) {
      GetLogger()->Log(
        cppmicroservices::logservice::SeverityLevel::LOG_WARNING,
        "Failure while trying to add reference to BoundServices Cache ");
      return;
    }
    try {
      instance->InvokeBindMethod(refName, ref);
    } catch (const std::exception&) {
      GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                       "Exception received from user code while binding a "
                       "service reference.",
                       std::current_exception());
    }
  }
}

void BundleOrPrototypeComponentConfigurationImpl::UnbindReference(
  const std::string& refName,
  const ServiceReferenceBase& ref)
{
  auto instancePairs = compInstanceMap.lock();
  for (auto const& instancePair : *instancePairs) {
    auto& instance = instancePair.first;
    auto& context = instancePair.second;
    try {
      instance->InvokeUnbindMethod(refName, ref);
    } catch (const std::exception&) {
      GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                       "Exception received from user code while unbinding a "
                       "service reference.",
                       std::current_exception());
    }

    context->RemoveFromBoundServicesCache(refName, ref);
  }
}

}
}
