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

#include "ReferenceManagerImpl.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices {
namespace scrimpl {

void ReferenceManagerBaseImpl::BindingPolicy::Log(
  std::string&& logStr,
  cppmicroservices::logservice::SeverityLevel logLevel)
{
  mgr.logger->Log(logLevel, logStr);
}

bool ReferenceManagerBaseImpl::BindingPolicy::ShouldClearBoundRefs(
  const ServiceReferenceBase& reference)
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  auto itr = boundRefsHandle->find(reference);
  return (itr != boundRefsHandle->end());
}

bool ReferenceManagerBaseImpl::BindingPolicy::ShouldNotifySatisfied()
{
  return (mgr.IsSatisfied() ? false : mgr.UpdateBoundRefs());
}

void ReferenceManagerBaseImpl::BindingPolicy::ClearBoundRefs()
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  boundRefsHandle->clear();
}

void ReferenceManagerBaseImpl::BindingPolicy::StaticRemoveService(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (ShouldClearBoundRefs(reference)) {
    Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
    RefChangeNotification notification{ mgr.metadata.name,
                                        RefEvent::BECAME_UNSATISFIED };
    notifications.push_back(std::move(notification));

    ClearBoundRefs();
    if (mgr.UpdateBoundRefs()) {
      Log("Notify SATISFIED for reference " + mgr.metadata.name);
      RefChangeNotification notification{ mgr.metadata.name,
                                          RefEvent::BECAME_SATISFIED };
      notifications.push_back(std::move(notification));
    }

    mgr.BatchNotifyAllListeners(notifications);
  }
}

void ReferenceManagerBaseImpl::BindingPolicy::DynamicRemoveService(
  const ServiceReferenceBase& reference)
{
  // OSGi Compendium Release 7 section 112.5.12 Bound Service Replacement
  //  If an active component configuration has a dynamic reference with unary
  //  cardinality and the bound service is modified or unregistered and ceases
  //  to be a target service, or the policy-option is greedy and a better
  //  target service becomes available then SCR must attempt to replace the
  //  bound service with a new bound service.
  auto removeBoundRef = false;
  std::vector<RefChangeNotification> notifications;

  { // acquire lock on boundRefs
    auto boundRefsHandle = mgr.boundRefs.lock();
    auto itr = boundRefsHandle->find(reference);
    removeBoundRef = (itr != boundRefsHandle->end());
  } // end lock on boundRefs

  if (removeBoundRef) {
    ClearBoundRefs();

    auto notifyRebind = mgr.UpdateBoundRefs();

    if (notifyRebind) {
      // The bind notification must happen before the unbind notification
      // to eliminate any gaps between unbinding the current bound target service
      // and binding to the new bound target service.
      ServiceReference<void> svcRefToBind;
      {
        auto boundRefsHandle =
          mgr.boundRefs.lock(); // acquires lock on boundRefs
        if (!boundRefsHandle->empty()) {
          svcRefToBind = *(boundRefsHandle->begin());
          Log("Notify BIND for reference " + mgr.metadata.name);
        }
      }

      Log("Notify UNBIND for reference " + mgr.metadata.name);
      RefChangeNotification notification{ mgr.metadata.name,
                                          RefEvent::REBIND,
                                          svcRefToBind,
                                          reference };
      notifications.push_back(std::move(notification));
    }

    if (!mgr.IsSatisfied()) {
      Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
      RefChangeNotification notification{ mgr.metadata.name,
                                          RefEvent::BECAME_UNSATISFIED };
      notifications.push_back(std::move(notification));
    }

    mgr.BatchNotifyAllListeners(notifications);
  }
}
}
}
