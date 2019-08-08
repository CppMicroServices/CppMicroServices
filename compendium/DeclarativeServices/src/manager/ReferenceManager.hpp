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

#ifndef __REFERENCEMANAGER_HPP__
#define __REFERENCEMANAGER_HPP__

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"

#include "../metadata/ReferenceMetadata.hpp"
#include "cppmicroservices/servicecomponent/detail/ComponentInstance.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices {
namespace scrimpl {
/**
 * Enum used to indicate the state of a {@link ReferenceManager}
 */
enum class RefEvent
{
  BECAME_SATISFIED,  /* used to notify the listener that the reference has
                        become satisfied */
  BECAME_UNSATISFIED,/* used to notify the listener that the reference has
                        become unsatisfied */
};

/**
 * This class is used by ReferenceManager to notify it's listener about the
 * state of the Reference.
 */
struct RefChangeNotification
{
  std::string senderName;
  RefEvent event;
};

/**
 * This is the interface for the ReferenceManager implementations.
 * This interface is consumed by ServiceComponentRuntimeImpl and
 * ComponentContextImpl classes to provide information about the
 * current state of a reference for a component.
 */
class ReferenceManager
{
public:
  ReferenceManager(const ReferenceManager&) = delete;
  ReferenceManager(ReferenceManager&&) = delete;
  ReferenceManager& operator=(const ReferenceManager&) = delete;
  ReferenceManager& operator=(ReferenceManager&&) = delete;
  virtual ~ReferenceManager() = default;

  /**
   * This method returns the name specified in the component
   * description for the reference managed by this object.
   */
  virtual std::string GetReferenceName() const = 0;

  /**
   * This method returns the service scope specified in the component
   * description for the reference managed by this object.
   */
  virtual std::string GetReferenceScope() const = 0;

  /**
   * This method returns the target string specified in the component
   * description for the reference managed by this object.
   */
  virtual std::string GetLDAPString() const = 0;

  /**
   * Returns true if the reference is satisfied, false otherwise.
   */
  virtual bool IsSatisfied() const = 0;

  /**
   * Returns true if the cardinality for this reference is "optional"
   */
  virtual bool IsOptional() const = 0;

  /**
   * Returns a set of ServiceReferences that are bound to the component.
   */
  virtual std::set<cppmicroservices::ServiceReferenceBase> GetBoundReferences() const = 0;

  /**
   * Returns a set of ServiceReferences that match the reference criteria but
   * are not bound to the component because the cardinality is not satisfied yet.
   */
  virtual std::set<cppmicroservices::ServiceReferenceBase> GetTargetReferences() const = 0;

  /**
   * Method is used to receive callbacks when the dependency is satisfied
   */
  virtual cppmicroservices::ListenerTokenId RegisterListener(std::function<void(const RefChangeNotification&)> notify) = 0;

  /**
   * Method is used to remove the callbacks registered using RegisterListener
   */
  virtual void UnregisterListener(cppmicroservices::ListenerTokenId token) = 0;

  /**
   * Method to stop tracking the reference service.
   */
  virtual void StopTracking() = 0;
protected:
  ReferenceManager() = default;
};
}
}
#endif // __REFERENCEMANAGER_HPP__
