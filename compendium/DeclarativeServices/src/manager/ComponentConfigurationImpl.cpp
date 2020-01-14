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

#include "ComponentConfigurationImpl.hpp"
#include <cassert>
#include <iostream>

#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "RegistrationManager.hpp"
#include "ReferenceManager.hpp"
#include "ReferenceManagerImpl.hpp"
#include "states/ComponentConfigurationState.hpp"
#include "states/CCUnsatisfiedReferenceState.hpp"
#include "BundleLoader.hpp"

using cppmicroservices::scrimpl::ReferenceManagerImpl;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_ID;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME;

namespace cppmicroservices { namespace scrimpl {

std::atomic<unsigned long> ComponentConfigurationImpl::idCounter(0);

ComponentConfigurationImpl::ComponentConfigurationImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata
                                                       , const Bundle& bundle
                                                       , std::shared_ptr<const ComponentRegistry> registry
                                                       , std::shared_ptr<cppmicroservices::logservice::LogService> logger)
  : configID(++idCounter)
  , metadata(std::move(metadata))
  , bundle(bundle)
  , registry(std::move(registry))
  , logger(std::move(logger))
  , state(std::make_shared<CCUnsatisfiedReferenceState>())
  , newCompInstanceFunc(nullptr)
  , deleteCompInstanceFunc(nullptr)
{
  if(!this->metadata || !this->bundle || !this->registry || !this->logger) {
    throw std::invalid_argument("ComponentConfigurationImpl - Invalid arguments passed to constructor");
  }
  
  auto const& serviceMetadata = this->metadata->serviceMetadata;
  if(!serviceMetadata.interfaces.empty()) {
    regManager = std::make_unique<RegistrationManager>(bundle.GetBundleContext(),
                                                       serviceMetadata.interfaces,
                                                       serviceMetadata.scope,
                                                       this->logger);
  }
  for (auto const& refMetadata : this->metadata->refsMetadata) {
    auto refManager = std::make_shared<ReferenceManagerImpl>(refMetadata,
                                                             bundle.GetBundleContext(),
                                                             this->logger,
                                                             this->metadata->name);
    referenceManagers.emplace(refMetadata.name, refManager);
  }
}

ComponentConfigurationImpl::~ComponentConfigurationImpl()
{
}

void ComponentConfigurationImpl::Stop()
{
  std::for_each(referenceManagerTokens.begin()
                , referenceManagerTokens.end()
                , [](const std::unordered_map<std::shared_ptr<ReferenceManager>, ListenerTokenId>::value_type& kvpair){
                    (kvpair.first)->UnregisterListener(kvpair.second);
                  });
  
  referenceManagerTokens.clear();
  for(auto& refMgr : referenceManagers) {
    refMgr.second->StopTracking();
  }
}

std::unordered_map<std::string, cppmicroservices::Any> ComponentConfigurationImpl::GetProperties() const {
  auto props = metadata->properties;
  props.emplace(COMPONENT_NAME, Any(this->metadata->name));
  props.emplace(COMPONENT_ID, Any(configID));
  return props;
}

void ComponentConfigurationImpl::Initialize()
{
  // Call Register if no dependencies exist
  // If dependencies exist, the dependency tracker mechanism will trigger the call to Register at the appropriate time.
  if(referenceManagers.empty()) {
    GetState()->Register(*this);
  }
  else {
    for(auto& kv : referenceManagers) {
      auto& refManager = kv.second;
      auto token = refManager->RegisterListener(std::bind(&ComponentConfigurationImpl::RefChangedState, this, std::placeholders::_1));
      referenceManagerTokens.emplace(refManager, token);
    }
  }
}

void ComponentConfigurationImpl::RefChangedState(const RefChangeNotification& notification)
{
  switch(notification.event) {
    case RefEvent::BECAME_SATISFIED:
      RefSatisfied(notification.senderName);
      break;
    case RefEvent::BECAME_UNSATISFIED:
      RefUnsatisfied(notification.senderName);
      break;
    default:
      break;
  }
}

std::vector<std::shared_ptr<ReferenceManager>> ComponentConfigurationImpl::GetAllDependencyManagers() const
{
  std::vector<std::shared_ptr<ReferenceManager>> refManagers;
  for(auto const& kv : referenceManagers) {
    refManagers.push_back(kv.second);
  }
  return refManagers;
}
std::shared_ptr<ReferenceManager> ComponentConfigurationImpl::GetDependencyManager(const std::string& refName) const
{
  return ((referenceManagers.count(refName) != 0u)
          ? referenceManagers.at(refName)
          : nullptr);
}

ServiceReferenceBase ComponentConfigurationImpl::GetServiceReference() const
{
  return (regManager
          ? regManager->GetServiceReference()
          : ServiceReferenceU());
}

bool ComponentConfigurationImpl::RegisterService()
{
  return (regManager
          ? regManager->RegisterService(GetFactory(), GetProperties())
          : false);
}

void ComponentConfigurationImpl::UnregisterService()
{
  if(regManager && regManager->IsServiceRegistered()) {
    regManager->UnregisterService();
  }
}

class SatisfiedFunctor
{
public:
  SatisfiedFunctor(std::string skipKeyName) : state(true), skipKey(std::move(skipKeyName)) {}
  SatisfiedFunctor(const SatisfiedFunctor& cpy) = default;
  SatisfiedFunctor(SatisfiedFunctor&& cpy) = default;
  SatisfiedFunctor& operator=(const SatisfiedFunctor& cpy) = default;
  SatisfiedFunctor& operator=(SatisfiedFunctor&& cpy) = default;
  ~SatisfiedFunctor() = default;
  bool IsSatisfied() const { return(state); }
  void operator () (const std::unordered_map<std::string, std::shared_ptr<ReferenceManager>>::value_type& item)
  {
    if(skipKey.empty() || item.first != skipKey) {
      state = state && item.second->IsSatisfied();
    }
  }
private:
  bool state;
  std::string skipKey;
};

void ComponentConfigurationImpl::RefSatisfied(const std::string& refName)
{
  SatisfiedFunctor f = std::for_each(referenceManagers.begin(), referenceManagers.end(), SatisfiedFunctor(refName));
  if(f.IsSatisfied()) {
    GetState()->Register(*this);
  }
}

void ComponentConfigurationImpl::RefUnsatisfied(const std::string& refName)
{
  if(referenceManagers.count(refName) != 0u) {
    // the state of the rest of the dependency managers is irrelevant.
    // deactivate the configuration
    GetState()->Deactivate(*this);
  }
}

void ComponentConfigurationImpl::Register()
{
  GetState()->Register(*this);
}

std::shared_ptr<ComponentInstance> ComponentConfigurationImpl::Activate(const Bundle& usingBundle)
{
  return GetState()->Activate(*this, usingBundle);
}

void ComponentConfigurationImpl::Deactivate()
{
  GetState()->Deactivate(*this);
}

ComponentState ComponentConfigurationImpl::GetConfigState() const
{
  return GetState()->GetValue();
}

bool ComponentConfigurationImpl::CompareAndSetState(std::shared_ptr<ComponentConfigurationState>* expectedState
                                                    , std::shared_ptr<ComponentConfigurationState> desiredState)
{
  return std::atomic_compare_exchange_strong(&state, expectedState, desiredState);
}


std::shared_ptr<ComponentConfigurationState> ComponentConfigurationImpl::GetState() const
{
  return std::atomic_load(&state);
}

void ComponentConfigurationImpl::LoadComponentCreatorDestructor()
{
  if(newCompInstanceFunc == nullptr || deleteCompInstanceFunc == nullptr) {
    std::tie(newCompInstanceFunc, deleteCompInstanceFunc) = GetComponentCreatorDeletors(GetMetadata()->implClassName, GetBundle());
  }
}

std::shared_ptr<ComponentInstance> ComponentConfigurationImpl::CreateComponentInstance()
{
  LoadComponentCreatorDestructor();
  return std::shared_ptr<ComponentInstance>(newCompInstanceFunc(), deleteCompInstanceFunc);
}

InstanceContextPair ComponentConfigurationImpl::CreateAndActivateComponentInstanceHelper(const cppmicroservices::Bundle& bundle)
{
  auto componentInstance = CreateComponentInstance();
  auto ctxt = std::make_shared<ComponentContextImpl>(shared_from_this(), bundle);
  componentInstance->CreateInstanceAndBindReferences(ctxt);
  componentInstance->Activate();
  return std::make_pair(componentInstance, ctxt);
}

}} // namespaces
