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

#include "BundleOrPrototypeComponentConfiguration.hpp"

namespace cppmicroservices {
namespace scrimpl {

BundleOrPrototypeComponentConfigurationImpl::BundleOrPrototypeComponentConfigurationImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                                                                                         const cppmicroservices::Bundle& bundle,
                                                                                         std::shared_ptr<const ComponentRegistry> registry,
                                                                                         std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : ComponentConfigurationImpl(metadata, bundle, registry, logger)
{
}

std::shared_ptr<ServiceFactory> BundleOrPrototypeComponentConfigurationImpl::GetFactory()
{
  auto thisPtr = std::dynamic_pointer_cast<BundleOrPrototypeComponentConfigurationImpl>(shared_from_this());
  return ToFactory(thisPtr);
}

BundleOrPrototypeComponentConfigurationImpl::~BundleOrPrototypeComponentConfigurationImpl()
{
  DestroyComponentInstances();
}

std::shared_ptr<ComponentInstance> BundleOrPrototypeComponentConfigurationImpl::CreateAndActivateComponentInstance(const cppmicroservices::Bundle& bundle)
{
  if(GetState()->GetValue() != service::component::runtime::dto::ACTIVE)
  {
    return nullptr;
  }
  auto compInstCtxtPairList = compInstanceMap.lock();
  try
  {
    auto instCtxtTuple = CreateAndActivateComponentInstanceHelper(bundle);
    compInstCtxtPairList->emplace_back(instCtxtTuple);
    return instCtxtTuple.first;
  }
  catch(...)
  {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                     "Exception received from user code while activating the component configuration",
                     std::current_exception());
  }
  return nullptr;
}

void BundleOrPrototypeComponentConfigurationImpl::DestroyComponentInstances()
{
  auto compInstCtxtPairList = compInstanceMap.lock();
  for(auto& valPair : *compInstCtxtPairList)
  {
    DeactivateComponentInstance(valPair);
  }
  compInstCtxtPairList->clear();
}

void BundleOrPrototypeComponentConfigurationImpl::DeactivateComponentInstance(const InstanceContextPair& instCtxt)
{
  try
  {
    instCtxt.first->Deactivate();
    instCtxt.first->UnbindReferences();
  }
  catch(...)
  {
    GetLogger()->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                     "Exception received from user code while deactivating the component configuration",
                     std::current_exception());
  }
  instCtxt.second->Invalidate();
}

InterfaceMapConstPtr BundleOrPrototypeComponentConfigurationImpl::GetService(const cppmicroservices::Bundle& bundle,
                                                                             const cppmicroservices::ServiceRegistrationBase& /*registration*/)
{
  // if activation passed, return the interface map from the instance
  auto compInstance = Activate(bundle);
  return compInstance ? compInstance->GetInterfaceMap() : nullptr;
}

void BundleOrPrototypeComponentConfigurationImpl::UngetService(const cppmicroservices::Bundle&,
                                                               const cppmicroservices::ServiceRegistrationBase& /*registration*/,
                                                               const cppmicroservices::InterfaceMapConstPtr& service)
{
  std::shared_ptr<void> obj = cppmicroservices::ExtractInterface(service, "");
  auto compInstCtxtPairList = compInstanceMap.lock();
  auto itr = std::find_if(compInstCtxtPairList->begin(), compInstCtxtPairList->end(), [&obj](const InstanceContextPair& instCtxtPair) {
      return (obj == cppmicroservices::ExtractInterface(instCtxtPair.first->GetInterfaceMap(), ""));
    });
  if(itr != compInstCtxtPairList->end())
  {
    DeactivateComponentInstance(*itr);
    compInstCtxtPairList->erase(itr);
  }
}
}
}

