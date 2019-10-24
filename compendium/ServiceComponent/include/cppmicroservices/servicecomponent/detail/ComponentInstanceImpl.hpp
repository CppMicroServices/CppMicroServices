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
#include <array>
#include <tuple>

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "ComponentInstance.hpp"
#include "Binders.hpp"

namespace cppmicroservices { namespace service { namespace component { namespace detail {

/**
 * Util class to detect if a class has a method named Activate
 * Member Detector Idiom - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 */
template <class T, class ReturnT, class... ArgsT>
class HasActivate
{
public:
  struct BadType {};
  
  template <class U>
  static decltype(std::declval<U>().Activate(std::declval<ArgsT>()...)) Test(int);
  
  template <class U>
  static BadType Test(...);
  
  static constexpr bool value = std::is_same<decltype(Test<T>(0)), ReturnT>::value;
};

/**
 * Util class to detect if a class has a method named Deactivate
 * Member Detector Idiom - https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Member_Detector
 */
template <class T, class ReturnT, class... ArgsT>
class HasDeactivate
{
public:
  struct BadType {};
  
  template <class U>
  static decltype(std::declval<U>().Deactivate(std::declval<ArgsT>()...)) Test(int);
  
  template <class U>
  static BadType Test(...);
  
  static constexpr bool value = std::is_same<decltype(Test<T>(0)), ReturnT>::value;
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

template <std::size_t I, class T>
using tuple_element_t = typename std::tuple_element<I, T>::type;

} // namespace util

template<typename... Ts, typename FUNC>
void for_each_in_tuple(std::tuple<Ts...> const& t, FUNC f)
{
  util::for_each(t, f, std::make_index_sequence<std::tuple_size<std::tuple<Ts...>>::value>{});
}

template <class T, class InterfaceTuple = std::tuple<>>
class ComponentInstanceImplBase
  : public ComponentInstance
{
public:

  ComponentInstanceImplBase(const std::vector<std::shared_ptr<Binder<T>>>& binders = {})
    : mContext(nullptr)
    , refBinders(binders)
  {
    // move all binders into a map
    for(size_t i = 0; i < refBinders.size(); i++)
    {
      refBinderMap[refBinders.at(i)->GetReferenceName()] = i;
    }
  }

  virtual ~ComponentInstanceImplBase() = default;

  void Activate() override
  {
    DoActivate(mContext);
  };

  void Deactivate() override
  {
    DoDeactivate(mContext);
  };

  void Modified() override { /* no-op for now */};

  void InvokeUnbindMethod(const std::string& refName
                          , const cppmicroservices::ServiceReferenceBase& sRef) override
  {
    size_t index = refBinderMap.at(refName);
    refBinders.at(index)->UnBind(mContext->GetBundleContext(), sRef, mServiceImpl);
  };

  void InvokeBindMethod(const std::string& refName
                        , const cppmicroservices::ServiceReferenceBase& sRef) override
  {
    size_t index = refBinderMap.at(refName);
    refBinders.at(index)->Bind(mContext->GetBundleContext(), sRef, mServiceImpl);
  };

  virtual std::shared_ptr<T> GetInstance() const { return mServiceImpl; };

  cppmicroservices::InterfaceMapPtr GetInterfaceMap() override
  {
    return GetInterfaceMapHelper(mServiceImpl);
  }

  template<class C = T, class I = InterfaceTuple, std::size_t... Is>
  cppmicroservices::InterfaceMapPtr MakeInterfaceMapWithTuple(const std::shared_ptr<T>& sObj
                                                              , std::index_sequence<Is...>)
  {
    // The static_ptr_cast inside MakeInterfaceMap will fail if the sObj does not implement any of the interfaces in I.
    InterfaceMapPtr iMap = MakeInterfaceMap<util::tuple_element_t<Is, I>...>(sObj);
    assert(iMap->size() == std::tuple_size<I>::value);
    return iMap;
  }

  template <class ...Args>
  cppmicroservices::InterfaceMapPtr GetInterfaceMapHelper(Args...)
  {
    return nullptr;
  }

  template <class I = InterfaceTuple, typename std::enable_if<std::tuple_size<I>::value != 0, int>::type = 0>
  cppmicroservices::InterfaceMapPtr GetInterfaceMapHelper(const std::shared_ptr<T>& sObj)
  {
    return MakeInterfaceMapWithTuple(sObj, std::make_index_sequence<std::tuple_size<I>::value>{});
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
  template <class Impl = T,
            class Z = typename std::enable_if<HasActivate<Impl, void, const std::shared_ptr<ComponentContext>&>::value>::type>
  void DoActivate(const std::shared_ptr<ComponentContext>& ctxt)
  {
    mServiceImpl->Activate(ctxt);
  }

  /**
   * This method is used if the component implementation class does not provide a Deactivate method.
   */
  template <typename ...A>
  void DoDeactivate(A...)
  {
    // no-op
  }

  /**
   * This method is used if the component implementation class provides a Deactivate method.
   */
  template <class Impl = T,
            class Z = typename std::enable_if<HasDeactivate<Impl, void, const std::shared_ptr<ComponentContext>&>::value>::type>
  void DoDeactivate(const std::shared_ptr<ComponentContext>& ctxt)
  {
    mServiceImpl->Deactivate(ctxt);
  }

  std::shared_ptr<ComponentContext> mContext;          // The context object associated with the component instance
  std::shared_ptr<T> mServiceImpl;                     // Instance of the implementation class
  std::vector<std::shared_ptr<Binder<T>>> refBinders;  // Helpers used for bind and unbind calls on the instance
  std::map<std::string, size_t> refBinderMap;          // a map to retrieve binders based on reference name
};


template <class T, class InterfaceTuple = std::tuple<>, class... CtorInjectedRefs>
class ComponentInstanceImpl final
  : public ComponentInstanceImplBase<T, InterfaceTuple>
{
public:
  ComponentInstanceImpl(const std::array<std::string, sizeof...(CtorInjectedRefs)> staticRefNames = {}
                          , const std::vector<std::shared_ptr<Binder<T>>>& binders = {})
    : ComponentInstanceImplBase<T, InterfaceTuple>(binders)
    , mStaticRefNames(staticRefNames)
  {
  }

  virtual ~ComponentInstanceImpl() = default;

  using Injection = std::integral_constant<bool, sizeof...(CtorInjectedRefs)!=0>;

  void CreateInstanceAndBindReferences(const std::shared_ptr<ComponentContext>& ctxt) override
  {
    this->mContext = ctxt;
    bool isConstructorInjected = Injection::value;
    std::shared_ptr<T> implObj = DoCreate(isConstructorInjected); // appropriate
    this->mServiceImpl = implObj;
    for(auto& binder : this->refBinders)
    {
      binder->Bind(ctxt, this->mServiceImpl);
    }
  }

  void UnbindReferences() override
  {
    for(auto& binder : this->refBinders)
    {
      binder->Unbind(this->mContext, this->mServiceImpl);
    }
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
  template <class C = T, class I = Injection,
            class X = typename std::enable_if<I::value == false>::type,
            class Y = typename std::enable_if<std::is_default_constructible<C>::value == true>::type>
  std::shared_ptr<T> DoCreate(bool)
  {
    return std::make_shared<T>();
  }

  // this method is used when injection is false and default constructor is not provided by the implementation class
  template <class C = T, class I = Injection,
            class X = typename std::enable_if<I::value == false>::type,
            class Y = typename std::enable_if<std::is_default_constructible<C>::value == false>::type>
  std::shared_ptr<T> DoCreate(bool, bool = false)
  {
    static_assert(std::is_default_constructible<C>::value, "Default Constructor expected when injection is false");
    return nullptr;
  }

  // this method is used when injection is true and constructor with suitable parameters is not provided by the implementation class
  template <class C = T, class I = Injection,
            class Y = typename std::enable_if<I::value == true>::type,
            class X = typename std::enable_if<std::is_constructible<C, const std::shared_ptr<CtorInjectedRefs>&...>::value == false>::type>
  std::shared_ptr<T> DoCreate(const bool&)
  {
    static_assert(std::is_constructible<C, const std::shared_ptr<CtorInjectedRefs>&...>::value , "Suitable constructor not found for constructor injection");
    return nullptr;
  }

  // this method is used when injection is true and constructor with parameters is provided by the implementation class
  template <class C = T, class I = Injection,
            class Y = typename std::enable_if<I::value == true>::type,
            class X = typename std::enable_if<std::is_constructible<C, const std::shared_ptr<CtorInjectedRefs>&...>::value>::type>
  std::shared_ptr<T> DoCreate(bool& injected)
  {
    std::tuple<std::shared_ptr<CtorInjectedRefs> ...> depObjs = GetAllDependencies(std::make_index_sequence<std::tuple_size<std::tuple<CtorInjectedRefs...>>::value>{});
    std::shared_ptr<T> implObj = call_make_shared_with_tuple(depObjs, std::make_index_sequence<std::tuple_size<std::tuple<std::shared_ptr<CtorInjectedRefs>...>>::value>{});
    injected = (implObj != nullptr);
    return implObj;
  }

  template<std::size_t... Is>
  std::shared_ptr<T> call_make_shared_with_tuple(const std::tuple<const std::shared_ptr<CtorInjectedRefs>&...>& tuple,
                                                 std::index_sequence<Is...>)
  {
    return std::make_shared<T>(std::get<Is>(tuple)...);
  }

  template<std::size_t... Is>
  std::tuple<std::shared_ptr<CtorInjectedRefs>...> GetAllDependencies(std::index_sequence<Is...>)
  {
    return std::make_tuple(GetDependency<typename std::tuple_element<Is, std::tuple<CtorInjectedRefs...>>::type>(std::get<Is>(mStaticRefNames))...);
  }

  template <class R>
  std::shared_ptr<R> GetDependency(const std::string& name)
  {
    return this->mContext->template LocateService<R>(name);
  }
private:
  std::array<std::string, sizeof...(CtorInjectedRefs)> mStaticRefNames;
};

}}}} // namespaces

#endif /* ComponentInstance_hpp */
