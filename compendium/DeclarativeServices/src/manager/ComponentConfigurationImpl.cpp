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
#include "../ConfigurationListenerImpl.hpp"
#include "BundleLoader.hpp"
#include "ComponentManager.hpp"
#include "ConfigurationManager.hpp"
#include "ReferenceManager.hpp"
#include "ReferenceManagerImpl.hpp"
#include "RegistrationManager.hpp"
#include "boost/asio/post.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "states/CCUnsatisfiedReferenceState.hpp"
#include "states/ComponentConfigurationState.hpp"
#include <cassert>
#include <iostream>
#include <memory>

using cppmicroservices::scrimpl::ReferenceManagerImpl;
using cppmicroservices::service::component::ComponentConstants::
  COMPONENT_FACTORY;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_ID;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME;
using cppmicroservices::service::component::ComponentConstants::
  CONFIG_POLICY_IGNORE;
using cppmicroservices::service::component::ComponentConstants::
  CONFIG_POLICY_OPTIONAL;

namespace cppmicroservices {
namespace scrimpl {

std::atomic<unsigned long> ComponentConfigurationImpl::idCounter(0);

ComponentConfigurationImpl::ComponentConfigurationImpl(
  std::shared_ptr<const metadata::ComponentMetadata> metadata,
  const Bundle& bundle,
  std::shared_ptr<ComponentRegistry> registry,
  std::shared_ptr<cppmicroservices::logservice::LogService> logger,
  std::shared_ptr<ConfigurationNotifier> configNotifier,
  std::shared_ptr<std::vector<std::shared_ptr<ComponentManager>>> managers)
  : configID(++idCounter)
  , metadata(std::move(metadata))
  , bundle(bundle)
  , registry(std::move(registry))
  , logger(std::move(logger))
  , configManager()
  , configNotifier(std::move(configNotifier))
  , managers(std::move(managers))
  , state(std::make_shared<CCUnsatisfiedReferenceState>())
  , newCompInstanceFunc(nullptr)
  , deleteCompInstanceFunc(nullptr)
{
  if (!this->metadata || !this->bundle || !this->registry || !this->logger ||
      !this->configNotifier || !this->managers) {
    throw std::invalid_argument(
      "ComponentConfigurationImpl - Invalid arguments passed to constructor");
  }

  auto const& serviceMetadata = this->metadata->serviceMetadata;
  if (!serviceMetadata.interfaces.empty()) {
    regManager =
      std::make_unique<RegistrationManager>(bundle.GetBundleContext(),
                                            serviceMetadata.interfaces,
                                            serviceMetadata.scope,
                                            this->logger);
  }
  for (auto const& refMetadata : this->metadata->refsMetadata) {

    auto refManager =
      std::make_shared<ReferenceManagerImpl>(refMetadata,
                                             bundle.GetBundleContext(),
                                             this->logger,
                                             this->metadata->name);
    referenceManagers.emplace(refMetadata.name, refManager);
  }
  if ((this->metadata->configurationPids.size() > 0) &&
      (this->metadata->configurationPolicy != CONFIG_POLICY_IGNORE)) {
    cppmicroservices::BundleContext bundleContext = bundle.GetBundleContext();
    configManager = std::make_shared<ConfigurationManager>(
      this->metadata, bundleContext, this->logger);
  }
}

ComponentConfigurationImpl::~ComponentConfigurationImpl() {}

void ComponentConfigurationImpl::Stop()
{
  std::for_each(
    referenceManagerTokens.begin(),
    referenceManagerTokens.end(),
    [](const std::unordered_map<std::shared_ptr<ReferenceManager>,
                                ListenerTokenId>::value_type& kvpair) {
      (kvpair.first)->UnregisterListener(kvpair.second);
    });

  referenceManagerTokens.clear();
  for (auto& refMgr : referenceManagers) {
    refMgr.second->StopTracking();
  }

  for (const auto& listener : configListenerTokens) {
    configNotifier->UnregisterListener(listener->pid, listener->tokenId);
  }
  configListenerTokens.clear();
}

std::unordered_map<std::string, cppmicroservices::Any>
ComponentConfigurationImpl::GetProperties() const
{
  if (metadata->factoryComponentID.empty()) {
    // This is not a factory component
    // Start with component properties
    auto props = metadata->properties;

    // If configuration object dependencies exist, use merged component and configuration object properties.
    if (configManager != nullptr) {
      props.clear();
      for (const auto& item : configManager->GetProperties()) {
        props.emplace(item.first, item.second);
      }
    }

    props.emplace(COMPONENT_NAME, Any(this->metadata->name));
    props.emplace(COMPONENT_ID, Any(configID));
    return props;
  } else {
    //This is  a factory component
    auto props = metadata->factoryComponentProperties;
    props.emplace(COMPONENT_NAME, Any(this->metadata->name));
    props.emplace(COMPONENT_FACTORY, Any(this->metadata->factoryComponentID));
    return props;
  }
}
void ComponentConfigurationImpl::SetRegistrationProperties()
{
  auto properties = GetProperties();
  if (regManager) {
    regManager->SetProperties(properties);
  }
}

void ComponentConfigurationImpl::Initialize()
{
  // Call Register if no dependencies exist
  // If dependencies exist, the dependency tracker mechanism will trigger the call to Register at the appropriate time.
  if (referenceManagers.empty() &&
      ((metadata->configurationPids.empty()) ||
       (metadata->configurationPolicy == CONFIG_POLICY_IGNORE))) {
    GetState()->Register(*this);
  } else {
    for (auto& kv : referenceManagers) {
      auto& refManager = kv.second;
      auto token = refManager->RegisterListener(
        std::bind(&ComponentConfigurationImpl::RefChangedState,
                  this,
                  std::placeholders::_1));
      referenceManagerTokens.emplace(refManager, token);
    }
    if (!metadata->configurationPids.empty() &&
        (metadata->configurationPolicy != CONFIG_POLICY_IGNORE)) {

      // Call RegisterListener to register listeners to listen for changes to configuration objects
      // before calling configManager->Initialize. The Initialize method will get the configuration object
      // from ConfigAdmin if it exists and a notification will be sent to the listeners.
      for (const auto& pid : metadata->configurationPids) {

        auto token = configNotifier->RegisterListener(
          pid,
          std::bind(&ComponentConfigurationImpl::ConfigChangedState,
                    this,
                    std::placeholders::_1),
          shared_from_this());
        auto listenerToken = std::make_shared<ListenerToken>(pid, token);
        configListenerTokens.emplace_back(listenerToken);
      }
      configManager->Initialize();
      if (referenceManagers.empty() && configManager->IsConfigSatisfied()) {
        GetState()->Register(*this);
      }
    }
  }
}

void ComponentConfigurationImpl::RefChangedState(
  const RefChangeNotification& notification)
{
  switch (notification.event) {
    case RefEvent::BECAME_SATISFIED:
      RefSatisfied(notification.senderName);
      break;
    case RefEvent::BECAME_UNSATISFIED:
      RefUnsatisfied(notification.senderName);
      break;
    case RefEvent::REBIND:
      GetState()->Rebind(*this,
                         notification.senderName,
                         notification.serviceRefToBind,
                         notification.serviceRefToUnbind);
      break;
    default:
      break;
  }
}
void ComponentConfigurationImpl::ConfigChangedState(
  const ConfigChangeNotification& notification)
{
  if (configManager == nullptr) {
    return;
  }
  bool configWasSatisfied = false;
  bool configNowSatisfied = false;

  configManager->UpdateMergedProperties(notification.pid,
                                        notification.newProperties,
                                        notification.event,
                                        configWasSatisfied,
                                        configNowSatisfied);

  if (configWasSatisfied && configNowSatisfied &&
      (metadata->configurationPolicy != CONFIG_POLICY_IGNORE)) {
    if (!Modified()) {
      //The Component does not have a Modified method so the component instance
      //has been deactivated.
      if (configManager->IsConfigSatisfied() && AreReferencesSatisfied()) {
        Register();
        return;
      }
    }
  }

  switch (notification.event) {
    case cppmicroservices::service::cm::ConfigurationEventType::CM_UPDATED:
      if (!configWasSatisfied && configNowSatisfied &&
          AreReferencesSatisfied()) {
        Register();
      }
      break;
    case cppmicroservices::service::cm::ConfigurationEventType::CM_DELETED:
      if (configWasSatisfied && !configNowSatisfied) {
        Deactivate();
      }
      break;
    default:
      break;
  }
}

std::vector<std::shared_ptr<ReferenceManager>>
ComponentConfigurationImpl::GetAllDependencyManagers() const
{
  std::vector<std::shared_ptr<ReferenceManager>> refManagers;
  for (auto const& kv : referenceManagers) {
    refManagers.push_back(kv.second);
  }
  return refManagers;
}
std::shared_ptr<ReferenceManager>
ComponentConfigurationImpl::GetDependencyManager(
  const std::string& refName) const
{
  return ((referenceManagers.count(refName) != 0u)
            ? referenceManagers.at(refName)
            : nullptr);
}

ServiceReferenceBase ComponentConfigurationImpl::GetServiceReference() const
{
  return (regManager ? regManager->GetServiceReference() : ServiceReferenceU());
}

bool ComponentConfigurationImpl::RegisterService()
{
  return (regManager
            ? regManager->RegisterService(GetFactory(), GetProperties())
            : false);
}

void ComponentConfigurationImpl::UnregisterService()
{
  if (regManager && regManager->IsServiceRegistered()) {
    regManager->UnregisterService();
  }
}

class SatisfiedFunctor
{
public:
  SatisfiedFunctor(std::string skipKeyName)
    : state(true)
    , skipKey(std::move(skipKeyName))
  {}
  SatisfiedFunctor(const SatisfiedFunctor& cpy) = default;
  SatisfiedFunctor(SatisfiedFunctor&& cpy) = default;
  SatisfiedFunctor& operator=(const SatisfiedFunctor& cpy) = default;
  SatisfiedFunctor& operator=(SatisfiedFunctor&& cpy) = default;
  ~SatisfiedFunctor() = default;
  bool IsSatisfied() const { return (state); }
  void operator()(
    const std::unordered_map<std::string,
                             std::shared_ptr<ReferenceManager>>::value_type&
      item)
  {
    if (skipKey.empty() || item.first != skipKey) {
      state = state && item.second->IsSatisfied();
    }
  }

private:
  bool state;
  std::string skipKey;
};

void ComponentConfigurationImpl::RefSatisfied(const std::string& refName)
{
  SatisfiedFunctor f = std::for_each(referenceManagers.begin(),
                                     referenceManagers.end(),
                                     SatisfiedFunctor(refName));
  if (configManager != nullptr) {
    if (!configManager->IsConfigSatisfied()) {
      return;
    }
  }
  if (f.IsSatisfied()) {
    GetState()->Register(*this);
  }
}

void ComponentConfigurationImpl::RefUnsatisfied(const std::string& refName)
{
  if (referenceManagers.count(refName) != 0u) {
    // the state of the rest of the dependency managers is irrelevant.
    // deactivate the configuration
    GetState()->Deactivate(*this);
  }
}
bool ComponentConfigurationImpl::AreReferencesSatisfied() const noexcept
{
  bool isSatisfied = true;

  for (const auto& mgr : referenceManagers) {
    if (!mgr.second->IsSatisfied()) {
      isSatisfied = false;
      break;
    }
  }

  return isSatisfied;
}

void ComponentConfigurationImpl::Register()
{
  GetState()->Register(*this);
}

std::shared_ptr<ComponentInstance> ComponentConfigurationImpl::Activate(
  const Bundle& usingBundle)
{
  return GetState()->Activate(*this, usingBundle);
}

void ComponentConfigurationImpl::Deactivate()
{
  GetState()->Deactivate(*this);
}

bool ComponentConfigurationImpl::Modified()
{
  return GetState()->Modified(*this);
}

ComponentState ComponentConfigurationImpl::GetConfigState() const
{
  // When DS is performing a state transition, it changes the state,
  // performs the action for the state transition, then signals that
  // the state transition is complete. Before returning the current
  // state to the caller, wait for the action associated with the
  // state to complete. For example, when registering a service,
  // DS changes the state to SATISFIED, registers the service and signals
  // that the state transition is complete. If the state of SATISFIED is
  // returned to the caller, it's important that the registration already
  // has taken place. Waiting for the transition to be signalled accomplishes
  // this.
  auto currentState = GetState();
  currentState->WaitForTransitionTask();
  return currentState->GetValue();
}

bool ComponentConfigurationImpl::CompareAndSetState(
  std::shared_ptr<ComponentConfigurationState>* expectedState,
  std::shared_ptr<ComponentConfigurationState> desiredState)
{
  return std::atomic_compare_exchange_strong(
    &state, expectedState, desiredState);
}

std::shared_ptr<ComponentConfigurationState>
ComponentConfigurationImpl::GetState() const
{
  return std::atomic_load(&state);
}

void ComponentConfigurationImpl::LoadComponentCreatorDestructor()
{
  if (newCompInstanceFunc == nullptr || deleteCompInstanceFunc == nullptr) {
    auto compName = GetMetadata()->name;
    auto position = compName.find("~");

    if ((position != std::string::npos)) {
      // this is a factory pid. Use implementation class
      // instead of name to construct the instance.
      compName = GetMetadata()->implClassName;
    } else {
      compName = GetMetadata()->name.empty() ? GetMetadata()->implClassName
                                             : GetMetadata()->name;
    }

    std::tie(newCompInstanceFunc, deleteCompInstanceFunc) =
      GetComponentCreatorDeletors(compName, GetBundle());
  }
}

std::shared_ptr<ComponentInstance>
ComponentConfigurationImpl::CreateComponentInstance()
{
  LoadComponentCreatorDestructor();
  return std::shared_ptr<ComponentInstance>(newCompInstanceFunc(),
                                            deleteCompInstanceFunc);
}

InstanceContextPair
ComponentConfigurationImpl::CreateAndActivateComponentInstanceHelper(
  const cppmicroservices::Bundle& bundle)
{
  auto componentInstance = CreateComponentInstance();
  auto ctxt =
    std::make_shared<ComponentContextImpl>(shared_from_this(), bundle);
  /*
   * Failing to construct the servce object is an unrecoverable 
   * failure which will cause the service configuration to not
   * be active.
   */
  try {
    componentInstance->CreateInstance(ctxt);
  } catch (const std::exception&) {
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                "Exception while creating component instance.",
                std::current_exception());
    throw;
  }

  /*
   * Binding service references can fail and if it does. per
   * the OSGi standard, the failure should be logged and the
   * activation should proceed.
   */
  try {
    componentInstance->BindReferences(ctxt);
  } catch (const std::exception&) {
    logger->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR,
                "Exception while binding references.",
                std::current_exception());
  }
  componentInstance->Activate();
  return std::make_pair(componentInstance, ctxt);
}
}
} // namespaces
