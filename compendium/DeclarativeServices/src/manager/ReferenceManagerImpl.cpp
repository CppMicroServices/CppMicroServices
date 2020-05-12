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
#include <cassert>
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "ReferenceManagerImpl.hpp"

using cppmicroservices::logservice::SeverityLevel;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME;
using cppmicroservices::service::component::ComponentConstants::REFERENCE_SCOPE_PROTOTYPE_REQUIRED;
using cppmicroservices::Constants::SERVICE_SCOPE;
using cppmicroservices::Constants::SCOPE_PROTOTYPE;

namespace cppmicroservices {
namespace scrimpl {

/**
 * @brief Returns the LDAPFilter of the reference metadata
 * @param refMetadata The metadata representing a service reference
 * @returns a LDAPFilter object corresponding to the @p refMetadata
 */
LDAPFilter GetReferenceLDAPFilter(const metadata::ReferenceMetadata& refMetadata)
{
  LDAPPropExpr expr;
  expr = (LDAPProp(cppmicroservices::Constants::OBJECTCLASS) == refMetadata.interfaceName);
  if(!refMetadata.target.empty())
  {
    expr &= LDAPPropExpr(refMetadata.target);
  }

  if(refMetadata.scope == REFERENCE_SCOPE_PROTOTYPE_REQUIRED)
  {
    expr &= (LDAPProp(SERVICE_SCOPE) ==  SCOPE_PROTOTYPE);
  }
  return LDAPFilter(expr);
}

ReferenceManagerImpl::ReferenceManagerImpl(const metadata::ReferenceMetadata& metadata,
                                           const cppmicroservices::BundleContext& bc,
                                           std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                           const std::string& configName)
  : metadata(metadata)
  , tracker(nullptr)
  , logger(std::move(logger))
  , configName(configName)
{
  if(!bc || !this->logger)
  {
    throw std::invalid_argument("Failed to create object, Invalid arguments passed to constructor");
  }
  try
  {
    tracker = std::make_unique<ServiceTracker<void>>(bc, GetReferenceLDAPFilter(metadata), this);
    tracker->Open();
  }
  catch(...)
  {
    logger->Log(SeverityLevel::LOG_ERROR, "could not open service tracker for " + metadata.interfaceName, std::current_exception());
    tracker.reset();
    throw std::current_exception();
  }
}

void ReferenceManagerImpl::StopTracking()
{
  try
  {
    tracker->Close();
  }
  catch(...)
  {
    logger->Log(SeverityLevel::LOG_ERROR, "Exception caught while closing service tracker for " + metadata.interfaceName, std::current_exception());
  }
}

std::set<cppmicroservices::ServiceReferenceBase> ReferenceManagerImpl::GetBoundReferences() const
{
  auto boundRefsHandle = boundRefs.lock();
  return std::set<cppmicroservices::ServiceReferenceBase>(boundRefsHandle->begin(), boundRefsHandle->end());
}

std::set<cppmicroservices::ServiceReferenceBase> ReferenceManagerImpl::GetTargetReferences() const
{
  auto matchedRefsHandle = matchedRefs.lock();
  return std::set<cppmicroservices::ServiceReferenceBase>(matchedRefsHandle->begin(), matchedRefsHandle->end());
}

// util method to extract service-id from a given reference
long GetServiceId(const ServiceReferenceBase& sRef)
{
  auto idAny = sRef.GetProperty(cppmicroservices::Constants::SERVICE_ID);
  return cppmicroservices::any_cast<long>(idAny);
}

bool ReferenceManagerImpl::IsOptional() const
{
  return (metadata.minCardinality == 0);
}

bool ReferenceManagerImpl::IsSatisfied() const
{
  return (boundRefs.lock()->size() >= metadata.minCardinality);
}

ReferenceManagerImpl::~ReferenceManagerImpl()
{
  StopTracking();
}

struct dummyRefObj {
};

bool ReferenceManagerImpl::UpdateBoundRefs()
{
  auto matchedRefsHandle = matchedRefs.lock(); // acquires lock on matchedRefs
  const auto matchedRefsHandleSize = matchedRefsHandle->size();
  if(matchedRefsHandleSize >= metadata.minCardinality)
  {
    auto boundRefsHandle = boundRefs.lock(); // acquires lock on boundRefs
    std::copy_n(matchedRefsHandle->rbegin(),
                std::min(metadata.maxCardinality, matchedRefsHandleSize),
                std::inserter(*(boundRefsHandle),
                              boundRefsHandle->begin()));
    return true;
  }
  return false;
  // release locks on matchedRefs and boundRefs
}

// This method implements the following algorithm
//
//  if reference becomes satisfied
//    Copy service references from #matchedRefs to #boundRefs
//    send a SATISFIED notification to listeners
//  else if reference is already satisfied
//    if policyOption is reluctant
//      ignore the new servcie
//    else if policyOption is GREEDY
//      if the new service is better than any of the existing services in #boundRefs
//        send UNSATISFIED notification to listeners
//        clear #boundRefs
//        copy #matchedRefs to #boundRefs
//        send a SATISFIED notification to listeners
//      endif
//    endif
//  endif
void ReferenceManagerImpl::ServiceAdded(const cppmicroservices::ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if(!reference)
  {
    logger->Log(SeverityLevel::LOG_DEBUG, "ServiceAdded: service with id " + std::to_string(GetServiceId(reference)) + " has already been unregistered, no-op");
    return;
  }
  // const auto minCardinality = metadata.minCardinality;
  // const auto maxCardinality = metadata.maxCardinality;
  // auto prevSatisfied = false;
  // auto becomesSatisfied = false;
  auto replacementNeeded = false;
  auto notifySatisfied = false;
  auto serviceIdToUnbind = -1;

  if(!IsSatisfied())
  {
    notifySatisfied = UpdateBoundRefs(); // becomes satisfied if return value is true
  }
  else // previously satisfied
  {
    if (metadata.policyOption == "greedy")
    {
      auto boundRefsHandle = boundRefs.lock(); // acquire lock on boundRefs
      if (boundRefsHandle->find(reference) == boundRefsHandle->end()) // reference is not bound yet
      {
        if (!boundRefsHandle->empty())
        {
          const ServiceReferenceBase& minBound = *(boundRefsHandle->begin());
          if (minBound < reference)
          {
            replacementNeeded = true;
            serviceIdToUnbind = GetServiceId(minBound);
          }
        }
        else
        {
          replacementNeeded = IsOptional();
        }
      }
    }
  }

  if(replacementNeeded)
  {
    logger->Log(SeverityLevel::LOG_DEBUG, "Notify UNSATISFIED for reference " + metadata.name);
    RefChangeNotification notification{metadata.name, RefEvent::BECAME_UNSATISFIED};
    notifications.push_back(std::move(notification));
    // The following "clear and copy" strategy is sufficient for
    // updating the boundRefs for static binding policy
    if(0 < serviceIdToUnbind)
    {
      auto boundRefsHandle = boundRefs.lock();
      boundRefsHandle->clear();
    }
    notifySatisfied = UpdateBoundRefs();
  }
  if(notifySatisfied)
  {
    logger->Log(SeverityLevel::LOG_DEBUG, "Notify SATISFIED for reference " + metadata.name);
    RefChangeNotification notification{metadata.name, RefEvent::BECAME_SATISFIED};
    notifications.push_back(std::move(notification));
  }
  BatchNotifyAllListeners(notifications);
}

cppmicroservices::InterfaceMapConstPtr ReferenceManagerImpl::AddingService(const cppmicroservices::ServiceReference<void>& reference)
{
  // Each service registered by DS contains a service property representing the component configuration name
  // to which it belongs. By checking the component configuration name of a service it can be determined
  // whether this service will satisfy its own reference. If it would, it is not a matched reference as
  // a service from the same component configuration can't satisfy its own reference.
  //
  // ASSUMPTION: If there is no component configuration name then its assumed this service was not registered by
  // DS and could not satisfy itself since it is not managed by DS.
  auto const compConfigName = reference.GetProperty(COMPONENT_NAME);
  if ((!compConfigName.Empty() && configName != compConfigName.ToStringNoExcept()) ||
    compConfigName.Empty()) { 
    // acquire lock on matchedRefs
    auto matchedRefsHandle = matchedRefs.lock();
    matchedRefsHandle->insert(reference);
  } // release lock on matchedRefs

  // After updating the bound references on this thread, notifying listeners happens on the same thread.
  // This behavior deviates from what is described in the "synchronous" section in
  // https://osgi.org/download/r6/osgi.core-6.0.0.pdf#page=432. Sporadically not returning a valid service
  // when a user calls getService, due to a service's references still resolving, was deemed undesirable
  // for user workflows.
  ServiceAdded(reference);

  // A non-null object must be returned to indicate to the ServiceTracker that
  // we are tracking the service and need to be called back when the service is removed.
  return MakeInterfaceMap<dummyRefObj>(std::make_shared<dummyRefObj>());
}

void ReferenceManagerImpl::ModifiedService(const cppmicroservices::ServiceReference<void>& /*reference*/,
                                           const cppmicroservices::InterfaceMapConstPtr& /*service*/)
{
  // no-op since there is no use case for property update
}

/**
 *This method implements the following algorithm
 *
 * If the removed service is found in the #boundRefs
 *   send a UNSATISFIED notification to listeners
 *   clear the #boundRefs member
 *   copy #matchedRefs to #boundRefs
 *   if reference is still satisfied
 *     send a SATISFIED notification to listeners
 *  endif
 * endif
 */
void ReferenceManagerImpl::ServiceRemoved(const cppmicroservices::ServiceReferenceBase& reference)
{
  auto removeBoundRef = false;
  std::vector<RefChangeNotification> notifications;

  { // acquire lock on boundRefs
    auto boundRefsHandle = boundRefs.lock();
    auto itr = boundRefsHandle->find(reference);
    removeBoundRef = (itr != boundRefsHandle->end());
  } // end lock on boundRefs

  if(removeBoundRef)
  {
    logger->Log(SeverityLevel::LOG_DEBUG, "Notify UNSATISFIED for reference " + metadata.name);
    RefChangeNotification notification { metadata.name, RefEvent::BECAME_UNSATISFIED };
    notifications.push_back(std::move(notification));
    {
      auto boundRefsHandle = boundRefs.lock();
      boundRefsHandle->clear();
    }
    auto notifySatisfied = UpdateBoundRefs();
    if(notifySatisfied)
    {
      logger->Log(SeverityLevel::LOG_DEBUG, "Notify SATISFIED for reference " + metadata.name);
      RefChangeNotification notification{metadata.name, RefEvent::BECAME_SATISFIED};
      notifications.push_back(std::move(notification));
    }
    BatchNotifyAllListeners(notifications);
  }
}

/**
 * If a target service is available to replace the bound service which became unavailable,
 * the component configuration must be reactivated and the replacement service is bound to
 * the new component instance.
 */
void ReferenceManagerImpl::RemovedService(const cppmicroservices::ServiceReference<void>& reference,
                                          const cppmicroservices::InterfaceMapConstPtr& /*service*/)
{
  { // acquire lock on matchedRefs
    auto matchedRefsHandle = matchedRefs.lock();
    matchedRefsHandle->erase(reference);
  } // release lock on matchedRefs

  // After updating the bound references on this thread, notifying listeners happens on the same thread.
  // This behavior deviates from what is described in the "synchronous" section in
  // https://osgi.org/download/r6/osgi.core-6.0.0.pdf#page=432. Sometimes not returning a valid service
  // due to a service's references still resolving was deemed undesirable for user workflows.
  ServiceRemoved(reference);
}

std::atomic<cppmicroservices::ListenerTokenId> ReferenceManagerImpl::tokenCounter(0);

/**
 * Method is used to register a listener for callbacks
 */
cppmicroservices::ListenerTokenId ReferenceManagerImpl::RegisterListener(std::function<void(const RefChangeNotification&)> notify)
{
  auto notifySatisfied = UpdateBoundRefs();
  if(notifySatisfied)
  {
    RefChangeNotification notification { metadata.name, RefEvent::BECAME_SATISFIED };
    notify(notification);
  }

  cppmicroservices::ListenerTokenId retToken = ++tokenCounter;
  {
    auto listenerMapHandle = listenersMap.lock();
    listenerMapHandle->emplace(retToken, notify);
  }
  return retToken;
}

/**
 * Method is used to remove a registered listener
 */
void ReferenceManagerImpl::UnregisterListener(cppmicroservices::ListenerTokenId token)
{
  auto listenerMapHandle = listenersMap.lock();
  listenerMapHandle->erase(token);
}

/**
 * Method used to notify all listeners
 */
void ReferenceManagerImpl::BatchNotifyAllListeners(const std::vector<RefChangeNotification>& notifications) noexcept
{
  if (notifications.empty() || listenersMap.lock()->empty())
  {
    return;
  }

  RefMgrListenerMap listenersMapCopy;
  {
    auto listenerMapHandle = listenersMap.lock();
    listenersMapCopy = *listenerMapHandle; // copy the listeners map
  }

  for(auto& listenerPair : listenersMapCopy)
  {
    for (auto const& notification : notifications)
    {
      listenerPair.second(notification);
    }
  }
}
}
}
