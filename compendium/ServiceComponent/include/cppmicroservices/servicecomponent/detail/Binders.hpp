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

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include <cppmicroservices/ServiceReference.h>

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace cppmicroservices
{
    namespace service
    {
        namespace component
        {
            namespace detail
            {

                /**
                 * Binder objects are used by the runtime to call the Bind and Unbind methods on a service component
                 * object.
                 */
                template <class T>
                class Binder
                {
                  public:
                    Binder(std::string const& refName, std::string const& refType)
                        : mRefName(refName)
                        , mRefType(refType)
                    {
                    }

                    virtual ~Binder() {};

                    virtual void Bind(std::shared_ptr<void> const& serviceToBind,
                                      std::shared_ptr<T> const& comp)
                        = 0;
                    virtual void UnBind(std::shared_ptr<void> const& serviceToUnbind,
                                        std::shared_ptr<T> const& comp)
                        = 0;
                    virtual void Bind(std::shared_ptr<ComponentContext> const& ctxt, std::shared_ptr<T> const& comp)
                        = 0;
                    virtual void Unbind(std::shared_ptr<ComponentContext> const& ctxt, std::shared_ptr<T> const& comp)
                        = 0;
                    virtual void Bind(std::shared_ptr<void> const& serv, std::shared_ptr<T> const& comp) = 0;
                    virtual void Unbind(std::shared_ptr<void> const& serv, std::shared_ptr<T> const& comp) = 0;
                    std::string
                    GetReferenceName()
                    {
                        return mRefName;
                    }
                    std::string
                    GetReferenceType()
                    {
                        return mRefType;
                    }

                  private:
                    std::string mRefName;
                    std::string mRefType;
                };

                /**
                 * This class is used to represent a binder object for a reference with static policy
                 */
                template <class T, class R>
                class StaticBinder final : public Binder<T>
                {
                  public:
                    StaticBinder(std::string const& refName) : Binder<T>(refName, us_service_interface_iid<R>()) {}
                    virtual ~StaticBinder() = default;
                    /**
                     * This method throws an exception to indicate an error condition to the runtime.
                     * References with static policy must not be modified throught out the lifetime
                     * of the service component object.
                     */
                    void
                    Bind(std::shared_ptr<void> const&,
                        std::shared_ptr<T> const&) override
                    {
                        throw std::runtime_error("Static dependency must not change at runtime");
                    }

                    /**
                     * This method throws an exception to indicate an error condition to the runtime.
                     * References with static policy must not be modified throught out the lifetime
                     * of the service component object.
                     */
                    void
                    UnBind(std::shared_ptr<void> const&,
                           std::shared_ptr<T> const&) override
                    {
                        throw std::runtime_error("Static dependency must not change at runtime");
                    }

                    void
                    Bind(std::shared_ptr<ComponentContext> const&, std::shared_ptr<T> const&) override
                    {
                        throw std::runtime_error("Static dependency must not change at runtime");
                    }

                    void
                    Unbind(std::shared_ptr<ComponentContext> const&, std::shared_ptr<T> const&) override
                    {
                        throw std::runtime_error("Static dependency must not change at runtime");
                    }

                    void
                    Bind(std::shared_ptr<void> const&, std::shared_ptr<T> const&) override
                    {
                        throw std::runtime_error("Static dependency must not change at runtime");
                    }

                    void
                    Unbind(std::shared_ptr<void> const&, std::shared_ptr<T> const&) override
                    {
                        throw std::runtime_error("Static dependency must not change at runtime");
                    }
                };

                /**
                 * This class is used to represent a binder object for a reference with dynamic policy
                 */
                template <class T, class R>
                class DynamicBinder final : public Binder<T>
                {
                  public:
                    using BindFuncT = std::function<void(T*, std::shared_ptr<R> const&)>;
                    using UnbindFuncT = std::function<void(T*, std::shared_ptr<R> const&)>;

                    DynamicBinder(std::string const& refName, BindFuncT bindFPtr, UnbindFuncT unbindFPtr)
                        : Binder<T>(refName, us_service_interface_iid<R>())
                        , bindFunction(bindFPtr)
                        , unbindFunction(unbindFPtr)
                    {
                    }

                    virtual ~DynamicBinder() = default;

                    void
                    Bind(std::shared_ptr<void> const& serviceToBind,
                         std::shared_ptr<T> const& comp) override
                    {
                        if (!serviceToBind)
                        {
                            throw std::runtime_error("Bind called on Invalid service");
                        }
                        DoBind(std::static_pointer_cast<R>(serviceToBind), comp);
                    }

                    void
                    UnBind(std::shared_ptr<void> const& serviceToUnbind,
                           std::shared_ptr<T> const& comp) override
                    {
                        if (!serviceToUnbind)
                        {
                            throw std::runtime_error("UnBind called on Invalid service");
                        }
                        DoUnbind(std::static_pointer_cast<R>(serviceToUnbind), comp);
                    }

                    void
                    Bind(std::shared_ptr<ComponentContext> const& ctxt, std::shared_ptr<T> const& comp) override
                    {
                        std::shared_ptr<R> service = ctxt->LocateService<R>(this->GetReferenceName());
                        DoBind(service, comp);
                    }

                    void
                    Unbind(std::shared_ptr<ComponentContext> const& ctxt, std::shared_ptr<T> const& comp) override
                    {
                        std::shared_ptr<R> service = ctxt->LocateService<R>(this->GetReferenceName());
                        DoUnbind(service, comp);
                    }

                    void
                    Bind(std::shared_ptr<void> const& serv, std::shared_ptr<T> const& comp) override
                    {
                        std::shared_ptr<R> service = std::static_pointer_cast<R>(serv);
                        DoBind(service, comp);
                    }

                    void
                    DoBind(std::shared_ptr<R> const& service, std::shared_ptr<T> const& comp)
                    {
                        auto bind = std::bind(bindFunction, comp.get(), service);
                        bind(); // call the method on the component instance with service as parameter.
                    }

                    void
                    Unbind(std::shared_ptr<void> const& serv, std::shared_ptr<T> const& comp) override
                    {
                        std::shared_ptr<R> service = std::static_pointer_cast<R>(serv);
                        DoUnbind(service, comp);
                    }

                    void
                    DoUnbind(std::shared_ptr<R> const& service, std::shared_ptr<T> const& comp)
                    {
                        auto unbind = std::bind(unbindFunction, comp.get(), service);
                        unbind(); // call the method on the component instance with service as parameter.
                    }

                  private:
                    BindFuncT bindFunction;     // the function object used for callback to bind the reference
                    UnbindFuncT unbindFunction; // the function object used for callback to unbind the reference
                };

            } // namespace detail
        } // namespace component
    } // namespace service
} // namespace cppmicroservices

#endif /* Binders_h */
