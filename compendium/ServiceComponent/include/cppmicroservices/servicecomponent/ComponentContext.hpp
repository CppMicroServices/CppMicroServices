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

#ifndef ComponentContext_hpp
#define ComponentContext_hpp

#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/servicecomponent/ServiceComponentExport.h"

namespace cppmicroservices { namespace service { namespace component {

/**
 * A Component Context object is used by a component instance to interact with
 * its execution context including locating services by reference name. Each
 * component instance has a unique Component Context.
 *
 * <p>
 * A component instance may obtain its Component Context object through its
 * activate, modified, and deactivate methods.
 */
class US_ServiceComponent_EXPORT ComponentContext
{
  public:
  virtual ~ComponentContext() noexcept;

  /**
   * Returns the component properties for this Component Context.
   *
   * @return The properties for this Component Context.
   */
  virtual std::unordered_map<std::string, cppmicroservices::Any> GetProperties() const = 0;

  /**
   * Returns the {@code BundleContext} of the bundle which contains this
   * component.
   *
   * @return The {@code BundleContext} of the bundle containing this
   *         component.
   */
  virtual cppmicroservices::BundleContext GetBundleContext() const = 0;

  /**
   * If the component instance is registered as a service using the
   * {@code servicescope="bundle"} or {@code servicescope="prototype"}
   * attribute, then this method returns the bundle using the service provided
   * by the component instance.
   * <p>
   * This method will return an invalid Bundle if:
   * <ul>
   * <li>The component instance is not a service, then no bundle can be using
   * it as a service.</li>
   * <li>The component instance is a service but did not specify the
   * {@code servicescope="bundle"} or {@code servicescope="prototype"}
   * attribute, then all bundles using the service provided by the component
   * instance will share the same component instance.</li>
   * <li>The service provided by the component instance is not currently being
   * used by any bundle.</li>
   * </ul>
   *
   * @return The bundle using the component instance as a service or
   *         an invalid bundle.
   */
  virtual cppmicroservices::Bundle GetUsingBundle() const = 0;

  /**
   * Enables the specified component name. The specified component name must
   * be in the same bundle as this component.
   *
   * <p>
   * This method must return after changing the enabled state of the specified
   * component name. Any actions that result from this, such as activating or
   * deactivating a component configuration, must occur asynchronously to this
   * method call.
   *
   * @param name The name of a component or empty string to indicate all
   *        components in the bundle.
   */
  virtual void EnableComponent(const std::string& name) = 0;

  /**
   * Disables the specified component name. The specified component name must
   * be in the same bundle as this component.
   *
   * <p>
   * This method must return after changing the enabled state of the specified
   * component name. Any actions that result from this, such as activating or
   * deactivating a component configuration, must occur asynchronously to this
   * method call.
   *
   * @param name The name of a component.
   */
  virtual void DisableComponent(const std::string& name) = 0;

  /**
   * If the component instance is registered as a service using the
   * {@code service} element, then this method returns the service reference
   * of the service provided by this component instance.
   * <p>
   * This method will return an invalid {@code ServiceReference} object if the
   * component instance is not registered as a service.
   *
   * @return The {@code ServiceReference} object for the component instance or
   *         invalid object if the component instance is not registered as a
   *         service.
   */
  virtual cppmicroservices::ServiceReferenceBase GetServiceReference() const = 0;

  /**
   * Returns the service object for the specified reference name and type.
   *
   * <p>
   * If the cardinality of the reference is {@code 0..n} or {@code 1..n} and
   * multiple services are bound to the reference, the service with the
   * highest ranking (as specified in its {@code Constants.SERVICE_RANKING}
   * property) is returned. If there is a tie in ranking, the service with the
   * lowest service id (as specified in its {@code Constants.SERVICE_ID}
   * property); that is, the service that was registered first is returned.
   *
   * @param name The name of a reference as specified in a {@code reference}
   *        element in this component's description.
   * @return A service object for the referenced service or {@code nullptr} if
   *         the reference cardinality is {@code 0..1} or {@code 0..n} and no
   *         bound service is available.
   * @throws ComponentException If Service Component Runtime catches an
   *         exception while activating the bound service.
   */
  template<class T>
  std::shared_ptr<T>  LocateService(const std::string& refName) const
  {
    std::shared_ptr<void> sObj = LocateService(refName, us_service_interface_iid<T>());
    return std::static_pointer_cast<T>(sObj);
  }

  /**
   * Returns the service objects for the specified reference name and type.
   *
   * @param name The name of a reference as specified in a {@code reference}
   *        element in this component's description.
   * @return A vector of service objects for the referenced service or
   *         empty vector if the reference cardinality is {@code 0..1} or
   *         {@code 0..n} and no bound service is available. If the reference
   *         cardinality is {@code 0..1} or {@code 1..1} and a bound service
   *         is available, the vector will have exactly one element.
   * @throws ComponentException If Service Component Runtime catches an
   *         exception while activating a bound service.
   */
  template<class T>
  std::vector<std::shared_ptr<T>>  LocateServices(const std::string& refName) const
  {
    auto sObjs = LocateServices(refName, us_service_interface_iid<T>());
    std::vector<std::shared_ptr<T>> objs;
    for(auto obj : sObjs)
    {
      objs.push_back(std::static_pointer_cast<T>(obj));
    }
    return objs;
  }

  protected:
  /**
   * Returns the service object for the specified reference name and type.
   *
   * <p>
   * If the cardinality of the reference is {@code 0..n} or {@code 1..n} and
   * multiple services are bound to the reference, the service with the
   * highest ranking (as specified in its {@code Constants.SERVICE_RANKING}
   * property) is returned. If there is a tie in ranking, the service with the
   * lowest service id (as specified in its {@code Constants.SERVICE_ID}
   * property); that is, the service that was registered first is returned.
   *
   * @param name The name of a reference as specified in a {@code reference}
   *        element in this component's description.
   * @return A service object for the referenced service or {@code nullptr} if
   *         the reference cardinality is {@code 0..1} or {@code 0..n} and no
   *         bound service is available.
   * @throws ComponentException If Service Component Runtime catches an
   *         exception while activating the bound service.
   */
  virtual std::shared_ptr<void> LocateService(const std::string& name, const std::string& type) const = 0;

  /**
   * Returns the service objects for the specified reference name and type.
   *
   * @param name The name of a reference as specified in a {@code reference}
   *        element in this component's description.
   * @return A vector of service objects for the referenced service or
   *         empty vector if the reference cardinality is {@code 0..1} or
   *         {@code 0..n} and no bound service is available. If the reference
   *         cardinality is {@code 0..1} or {@code 1..1} and a bound service
   *         is available, the vector will have exactly one element.
   * @throws ComponentException If Service Component Runtime catches an
   *         exception while activating a bound service.
   */
  virtual  std::vector<std::shared_ptr<void>> LocateServices(const std::string& name, const std::string& type) const = 0;
};

}}} // namespaces

#endif /* ComponentContext_hpp */
