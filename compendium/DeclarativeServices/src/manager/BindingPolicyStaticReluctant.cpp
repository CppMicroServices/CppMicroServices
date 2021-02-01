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
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"

namespace cppmicroservices {
namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyStaticReluctant::ServiceAdded(
  const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (!reference) {
    Log("BindingPolicyStaticReluctant::ServiceAdded called with an invalid "
        "service reference");
    return;
  }
  auto notifySatisfied = ShouldNotifySatisfied();
  if (notifySatisfied) {
    Log("Notify SATISFIED for reference " + mgr.metadata.name);
    notifications.emplace_back(
      mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference);
  }

  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyStaticReluctant::ServiceRemoved(
  const ServiceReferenceBase& reference)
{
  StaticRemoveService(reference);
}

}
} // namespaces
