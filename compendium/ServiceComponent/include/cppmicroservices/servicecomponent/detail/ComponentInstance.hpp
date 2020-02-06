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

#ifndef ComponentInstance_hpp
#define ComponentInstance_hpp
#include <memory>
#include <map>

#include <cppmicroservices/ServiceReference.h>
#include "../ComponentContext.hpp"
#include "cppmicroservices/servicecomponent/ServiceComponentExport.h"

namespace cppmicroservices { namespace service { namespace component { namespace detail {

/**
 * This interface is used by the declarative services runtime to manage the
 * creation, dependency injection and deletion of instances of the service
 * component class.
 */
class US_ServiceComponent_EXPORT ComponentInstance {
  public:
  virtual ~ComponentInstance() noexcept;

  /**
   * This method is responsible for creating an instance of the service component
   * implementation class. The dependencies are injected into the class constructor.
   * This method is called by the runtime while activating the component configuration.
   *
   * @param ctxt The {@code ComponentContext} object associated with the component instance.
   */
  virtual void CreateInstanceAndBindReferences(const std::shared_ptr<ComponentContext>& ctxt) = 0;
  virtual void UnbindReferences() = 0;
  
  /**
   * This method is called by the runtime while activating the component configuration.
   * It is called after the call to {@code #CreateInstanceAndBindReferences} method suceeded.
   */
  virtual void Activate() = 0;

  /**
   * This method is called by the runtime while deactivating the component configuration.
   * On return, the service component object is destroyed.
   */
  virtual void Deactivate() = 0;

  /**
   * This method is called by the runtime while the component configuration is active and if
   * the configuration properties are modified.
   */
  virtual void Modified() = 0;

  /**
   * This method is called by the runtime to bind a reference with dynamic policy
   */
  virtual void InvokeUnbindMethod(const std::string& refName, const cppmicroservices::ServiceReferenceBase& sRef) = 0;

  /**
   * This method is called by the runtime to unbind a reference with dynamic policy
   */
  virtual void InvokeBindMethod(const std::string& refName, const cppmicroservices::ServiceReferenceBase& sRef) = 0;

  /**
   * This method is called when a call to @{code ServiceFactory#GetService} is received by the runtime.
   */
  virtual cppmicroservices::InterfaceMapPtr GetInterfaceMap() = 0;
};

}}}} // namespaces

#endif /* ComponentInstance_hpp */
