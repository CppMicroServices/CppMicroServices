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

#ifndef ComponentWrapperImpl_hpp
#define ComponentWrapperImpl_hpp
#include <cassert>
#include <memory>
#include <vector>

#include "../ComponentContext.hpp"
#include "ComponentInstance.hpp"
#include "Binders.hpp"

namespace cppmicroservices { namespace service { namespace component { namespace detail {

/**
 * Util class to detect if a class has a method named Activate
 * Member Detector Idiom - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 */
template<typename T, typename R = void, typename ...Args>
class HasActivate
{
  template <typename U, R (U::*)(Args...)> struct Check;
  template <typename U> static char func(Check<U, &U::Activate> *);
  template <typename U> static int func(...);
public:
  typedef HasActivate type;
  enum { value = sizeof(func<T>(nullptr)) == sizeof(char) };
};

/**
 * Util class to detect if a class has a method named Deactivate
 * Member Detector Idiom - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 */
template<typename T, typename R = void, typename ...Args>
class HasDeactivate
{
  template <typename U, R (U::*)(Args...)> struct Check;
  template <typename U> static char func(Check<U, &U::Deactivate> *);
  template <typename U> static int func(...);
public:
  typedef HasDeactivate type;
  enum { value = sizeof(func<T>(nullptr)) == sizeof(char) };
};

namespace util {

/**
 * Helper function used to call a given function for each element of the tuple
 */
template<typename T, typename FUNC, std::size_t... Is>
void for_each(T&& t, FUNC f, std::index_sequence<Is...>)
{
  // we use the comma operator to execute the function after the parameters are unpacked.
  int dummy[] = {0, (f(std::get<Is>(t), Is), 0)...};
  static_cast<void>(dummy); // silence unused varible warning
}

} // util namespace

template<typename... Ts, typename FUNC>
void for_each_in_tuple(std::tuple<Ts...> const& t, FUNC f)
{
  util::for_each(t, f, std::make_index_sequence<std::tuple_size<std::tuple<Ts...>>::value>{});
}

/**
 * This class implements the private interface ComponentInstance. Extern C functions
 * responsible for instantiating objects from this class are generated using the code
 * generator tool.
 *
 * T - service component implementation class
 * 
 * InterfaceTuple - a tuple containing the interface types of the services provided by the
 *                  service component
 * 
 * Inject - indicates if the service component wants it's dependencies injected. Default
 *          is true.  When Inject is true, all references (static & dynamic) are injected
 *          using constructor injection.  If a reference has dynamic policy, in addition
 *          to constructor injection, the bind and unbind callbacks maybe called by the
 *          runtime to update the reference.  When Inject is false, no references are
 *          injected. Service component implementation object is responsible for
 *          retrieving its dependencies.
 *          
 * Refs - variadic parameter to specify the service reference types consumed by the
 *        component
 */
template <class T
          , class InterfaceTuple = std::tuple<>
          , class Inject = std::true_type
          , class ...Refs>
class ComponentInstanceImpl final
  : public ComponentInstance
{
public:
  ComponentInstanceImpl(const std::vector<std::shared_ptr<Binder<T>>>& binders = {})
    : mContext(nullptr)
    , refBinders(binders)
  {
    // move all binders into a map
    for(size_t i = 0; i < refBinders.size(); i++)
    {
      refBinderMap[refBinders.at(i)->GetReferenceName()] = i;
    }
  }

  virtual ~ComponentInstanceImpl() = default;

  virtual void CreateInstanceAndBindReferences(const std::shared_ptr<ComponentContext>& ctxt) override
  {
    mContext = ctxt;
    bool isConstructorInjected = false;
    std::shared_ptr<T> implObj = nullptr;
    implObj = DoCreate(isConstructorInjected); // appropriate overload is used at runtime
    mServiceImpl = implObj;
  }

  virtual void Activate() override
  {
    DoActivate(mContext);
  };

  virtual void Deactivate() override
  {
    DoDeactivate(mContext);
  };

  virtual void Modified() override { /* no-op for now */};

  virtual void InvokeUnbindMethod(const std::string& refName, const cppmicroservices::ServiceReferenceBase& sRef) override
  {
    size_t index = refBinderMap.at(refName);
    refBinders.at(index)->UnBind(mContext->GetBundleContext(), sRef, mServiceImpl);
  };

  virtual void InvokeBindMethod(const std::string& refName, const cppmicroservices::ServiceReferenceBase& sRef) override
  {
    size_t index = refBinderMap.at(refName);
    refBinders.at(index)->Bind(mContext->GetBundleContext(), sRef, mServiceImpl);
  };

  virtual std::shared_ptr<T> GetInstance() const { return mServiceImpl; };

  virtual cppmicroservices::InterfaceMapPtr GetInterfaceMap() override
  {
    return GetInterfaceMapHelper(mServiceImpl);
  }

private:

  template<std::size_t... Is>
  std::shared_ptr<T> call_make_shared_with_tuple(const std::tuple<const std::shared_ptr<Refs>&...>& tuple,
                                                 std::index_sequence<Is...>)
  {
    return std::make_shared<T>(std::get<Is>(tuple)...);
  }

  template<std::size_t... Is>
  std::tuple<std::shared_ptr<Refs>...> GetAllDependencies(std::index_sequence<Is...>)
  {
    return std::make_tuple(GetDependency<typename std::tuple_element<Is, std::tuple<Refs...>>::type>(refBinders.at(Is)->GetReferenceName())...);
  }

  template <class R>
  std::shared_ptr<R> GetDependency(const std::string& name)
  {
    return mContext->LocateService<R>(name);
  }

  template<class C = T
           , class I = InterfaceTuple
           , std::size_t... Is>
  cppmicroservices::InterfaceMapPtr MakeInterfaceMapWithTuple(const std::shared_ptr<T>& sObj,
                                                              std::index_sequence<Is...>)
  {
    // The static_ptr_cast inside MakeInterfaceMap will fail if the sObj does not
    // implement any of the interfaces in I.
    InterfaceMapPtr iMap = MakeInterfaceMap<typename std::tuple_element<Is, I>::type ...>(sObj);
    assert(iMap->size() == std::tuple_size<I>::value);
    return iMap;
  }

  template <class ...Args>
  cppmicroservices::InterfaceMapPtr GetInterfaceMapHelper(Args...)
  {
    return nullptr;
  }

  template <class I = InterfaceTuple
            , typename std::enable_if<std::tuple_size<I>::value != 0, int>::type = 0>
  cppmicroservices::InterfaceMapPtr GetInterfaceMapHelper(const std::shared_ptr<T>& sObj)
  {
    return MakeInterfaceMapWithTuple(sObj, std::make_index_sequence<std::tuple_size<I>::value>{});
  }

  /**
   * DoCreate is a helper function used to invoke the appropriate constructor on the Implementation class.
   * SFINAE is used to determine which overload of DoCreate is used by the runtime.
   */
  template <typename ...A>
  std::shared_ptr<T> DoCreate(A...)
  {
    // no-op fallback to mute the compiler.
    // There is always an overload with higher precedence available.
    return nullptr;
  }

  // this method is used when injection is false and default constructor is provided by the implementation class
  template <class C = T
            , class I = Inject
            , class X = typename std::enable_if<I::value == false>::type
            , class Y = typename std::enable_if<std::is_default_constructible<C>::value == true>::type>
  std::shared_ptr<T> DoCreate(bool)
  {
    return std::make_shared<T>();
  }

  // this method is used when injection is false and default constructor is not provided by the implementation class
  template <class C = T
            , class I = Inject
            , class X = typename std::enable_if<I::value == false>::type
            , class Y = typename std::enable_if<std::is_default_constructible<C>::value == false>::type>
  std::shared_ptr<T> DoCreate(bool dummy1, bool dummy2 = false)
  {
    (void)dummy1;
    (void)dummy2;
    static_assert(std::is_default_constructible<C>::value, "Default Constructor expected when injection is false");
    return nullptr;
  }

  // this method is used when injection is true and constructor with suitable parameters is not provided by the implementation class
  template <class C = T
            , class I = Inject
            , class Y = typename std::enable_if<I::value == true>::type
            , class X = typename std::enable_if<std::is_constructible<C, const std::shared_ptr<Refs>&...>::value == false>::type>
  std::shared_ptr<T> DoCreate(const bool&)
  {
    static_assert(std::is_constructible<C, const std::shared_ptr<Refs>&...>::value , "Suitable constructor not found for constructor injection");
    return nullptr;
  }

  // this method is used when injection is true and constructor with parameters is provided by the implementation class
  template <class C = T
            , class I = Inject
            , class Y = typename std::enable_if<I::value == true>::type
            , class X = typename std::enable_if<std::is_constructible<C, const std::shared_ptr<Refs>&...>::value>::type>
  std::shared_ptr<T> DoCreate(bool& injected)
  {
    std::tuple<std::shared_ptr<Refs> ...> depObjs = GetAllDependencies(std::make_index_sequence<std::tuple_size<std::tuple<Refs...>>::value>{});
    std::shared_ptr<T> implObj = call_make_shared_with_tuple(depObjs, std::make_index_sequence<std::tuple_size<std::tuple<std::shared_ptr<Refs>...>>::value>{});
    injected = (implObj != nullptr);
    return implObj;
  }

  /**
   * This method is used if the component implementation class does not provide an Activate method.
   */
  template <typename ...A>
  void DoActivate(A...)
  {
    // no-op
  }

  /**
   * This method is used if the component implementation class provides an Activate method.
   */
  template <class Impl = T
            , class Z = typename std::enable_if<HasActivate<Impl, void, const std::shared_ptr<ComponentContext>&>::value>::type>
  void DoActivate(const std::shared_ptr<ComponentContext>& ctxt)
  {
    mServiceImpl->Activate(ctxt);
  }

  /**
   * This method is used if the component implementation class does not provide an Activate method.
   */
  template <typename ...A>
  void DoDeactivate(A...)
  {
    // no-op
  }

  /**
   * This method is used if the component implementation class provides an Activate method.
   */
  template <class Impl = T
            , class Z = typename std::enable_if<HasDeactivate<Impl, void, const std::shared_ptr<ComponentContext>&>::value>::type>
  void DoDeactivate(const std::shared_ptr<ComponentContext>& ctxt)
  {
    mServiceImpl->Deactivate(ctxt);
  }

  std::shared_ptr<ComponentContext> mContext;          // The context object associated with the component instance
  std::shared_ptr<T> mServiceImpl;                     // Instance of the implementation class
  std::vector<std::shared_ptr<Binder<T>>> refBinders;  // Helpers used for bind and unbind calls on the instance
  std::map<std::string, size_t> refBinderMap;          // a map to retrieve binders based on reference name
};

}}}} // namespaces

#endif /* ComponentInstance_hpp */
