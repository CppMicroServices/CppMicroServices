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

#ifndef Binders_hpp
#define Binders_hpp

#include <memory>
#include <vector>

#include <cppmicroservices/ServiceReference.h>

namespace cppmicroservices { namespace service { namespace component { namespace detail {

/**
 * Binder objects are used by the runtime to call the Bind and Unbind methods on a service component object.
 */
template <class T>
class Binder
{
public:
  Binder(const std::string& refName,
         const std::string& refType)
    : mRefName(refName)
    , mRefType(refType)
  {}
  
  virtual ~Binder() {};
  
  virtual void Bind(cppmicroservices::BundleContext bc,
                    const cppmicroservices::ServiceReferenceBase& sRef,
                    const std::shared_ptr<T>& comp) = 0;
  virtual void UnBind(cppmicroservices::BundleContext bc,
                      const cppmicroservices::ServiceReferenceBase& sRef,
                      const std::shared_ptr<T>& comp) = 0;
  virtual void Bind(const std::shared_ptr<ComponentContext>& ctxt,
                    const std::shared_ptr<T>& comp) = 0;
  virtual void Unbind(const std::shared_ptr<ComponentContext>& ctxt,
                      const std::shared_ptr<T>& comp) = 0;
  virtual void Bind(const std::shared_ptr<void>& serv,
                    const std::shared_ptr<T>& comp) = 0;
  virtual void Unbind(const std::shared_ptr<void>& serv,
                      const std::shared_ptr<T>& comp) = 0;
  std::string GetReferenceName() { return mRefName; }
  std::string GetReferenceType() { return mRefType; }
private:
  std::string mRefName;
  std::string mRefType;
};

/**
 * This class is used to represent a binder object for a reference with static policy
 */
template <class T, class R>
class StaticBinder final
  : public Binder<T>
{
public:
  StaticBinder(const std::string& refName)
    : Binder<T>(refName, us_service_interface_iid<R>())
  {}
  virtual ~StaticBinder() = default;
  /**
   * This method throws an exception to indicate an error condition to the runtime.
   * References with static policy must not be modified throught out the lifetime
   * of the service component object.
   */
  void Bind(cppmicroservices::BundleContext
            , const cppmicroservices::ServiceReferenceBase&
            , const std::shared_ptr<T>&) override
  {
    throw std::runtime_error("Static dependency must not change at runtime");
  }

  /**
   * This method throws an exception to indicate an error condition to the runtime.
   * References with static policy must not be modified throught out the lifetime
   * of the service component object.
   */
  void UnBind(cppmicroservices::BundleContext
              , const cppmicroservices::ServiceReferenceBase&
              , const std::shared_ptr<T>&) override
  {
    throw std::runtime_error("Static dependency must not change at runtime");
  }

  void Bind(const std::shared_ptr<ComponentContext>&
            , const std::shared_ptr<T>&) override
  {
    throw std::runtime_error("Static dependency must not change at runtime");
  }
  
  void Unbind(const std::shared_ptr<ComponentContext>&
              , const std::shared_ptr<T>&) override
  {
    throw std::runtime_error("Static dependency must not change at runtime");
  }

  void Bind(const std::shared_ptr<void>&
            , const std::shared_ptr<T>&) override
  {
    throw std::runtime_error("Static dependency must not change at runtime");
  }

  void Unbind(const std::shared_ptr<void>&
              , const std::shared_ptr<T>&) override
  {
    throw std::runtime_error("Static dependency must not change at runtime");
  }
};

/**
 * This class is used to represent a binder object for a reference with dynamic policy
 */
template <class T, class R>
class DynamicBinder final
  : public Binder<T>
{
public:

  using BindFuncT = std::function<void(T*,const std::shared_ptr<R>&)>;
  using UnbindFuncT = std::function<void(T*,const std::shared_ptr<R>&)>;

  DynamicBinder(const std::string& refName
                , BindFuncT bindFPtr
                , UnbindFuncT unbindFPtr)
    : Binder<T>(refName, us_service_interface_iid<R>())
    , bindFunction(bindFPtr)
    , unbindFunction(unbindFPtr)
  {}

  virtual ~DynamicBinder() = default;

  void Bind(cppmicroservices::BundleContext bc
            , const cppmicroservices::ServiceReferenceBase& sRef
            , const std::shared_ptr<T>& comp) override
  {
    cppmicroservices::ServiceReference<R> typedRef(sRef);
    if(!typedRef)
    {
      throw std::runtime_error("Invalid service reference");
    }
    std::shared_ptr<R> service = bc.template GetService<R>(typedRef);
    DoBind(service, comp);
  }

  void UnBind(cppmicroservices::BundleContext bc
              , const cppmicroservices::ServiceReferenceBase& sRef
              , const std::shared_ptr<T>& comp) override
  {
    cppmicroservices::ServiceReference<R> typedRef(sRef);
    if(!typedRef)
    {
      throw std::runtime_error("Invalid service reference");
    }
    std::shared_ptr<R> service = bc.template GetService<R>(typedRef);
    DoUnbind(service, comp);
  }

  void Bind(const std::shared_ptr<ComponentContext>& ctxt
            , const std::shared_ptr<T>& comp) override
  {
    std::shared_ptr<R> service = ctxt->LocateService<R>(this->GetReferenceName());
    DoBind(service, comp);
  }
  
  void Unbind(const std::shared_ptr<ComponentContext>& ctxt
              , const std::shared_ptr<T>& comp) override
  {
    std::shared_ptr<R> service = ctxt->LocateService<R>(this->GetReferenceName());
    DoUnbind(service, comp);
  }

  void Bind(const std::shared_ptr<void>& serv
            , const std::shared_ptr<T>& comp) override
  {
    std::shared_ptr<R> service = std::static_pointer_cast<R>(serv);
    DoBind(service, comp);
  }

  void DoBind(const std::shared_ptr<R>& service
              , const std::shared_ptr<T>& comp)
  {
    auto bind = std::bind(bindFunction, comp.get(), service);
    bind(); // call the method on the component instance with service as parameter.
  }

  void Unbind(const std::shared_ptr<void>& serv
              , const std::shared_ptr<T>& comp) override
  {
    std::shared_ptr<R> service = std::static_pointer_cast<R>(serv);
    DoUnbind(service, comp);
  }

  void DoUnbind(const std::shared_ptr<R>& service
                , const std::shared_ptr<T>& comp)
  {
    auto unbind = std::bind(unbindFunction, comp.get(), service);
    unbind(); // call the method on the component instance with service as parameter.
  }

private:
  BindFuncT bindFunction;      // the function object used for callback to bind the reference
  UnbindFuncT unbindFunction;  // the function object used for callback to unbind the reference
};

}}}} // namespaces

#endif /* Binders_h */
