/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef USLISTENERFUNCTORS_P_H
#define USLISTENERFUNCTORS_P_H

#include <usConfig.h>
#include <usServiceEvent.h>
#include <usModuleEvent.h>

#include <algorithm>
#include <cstring>

#if defined(US_USE_CXX11) || defined(__GNUC__)

  #ifdef US_USE_CXX11
    #include <functional>
    #define US_MODULE_LISTENER_FUNCTOR std::function<void(const US_PREPEND_NAMESPACE(ModuleEvent)&)>
    #define US_SERVICE_LISTENER_FUNCTOR std::function<void(const US_PREPEND_NAMESPACE(ServiceEvent)&)>
  #else
    #include <tr1/functional>
    #define US_MODULE_LISTENER_FUNCTOR std::tr1::function<void(const US_PREPEND_NAMESPACE(ModuleEvent)&)>
    #define US_SERVICE_LISTENER_FUNCTOR std::tr1::function<void(const US_PREPEND_NAMESPACE(ServiceEvent)&)>
  #endif

  US_BEGIN_NAMESPACE
    template<class X>
    US_MODULE_LISTENER_FUNCTOR ModuleListenerMemberFunctor(X* x, void (X::*memFn)(const US_PREPEND_NAMESPACE(ModuleEvent)))
    { return std::bind1st(std::mem_fun(memFn), x); }
  US_END_NAMESPACE

  US_BEGIN_NAMESPACE
    struct ModuleListenerCompare : std::binary_function<std::pair<US_MODULE_LISTENER_FUNCTOR, void*>,
                                                        std::pair<US_MODULE_LISTENER_FUNCTOR, void*>, bool>
    {
      bool operator()(const std::pair<US_MODULE_LISTENER_FUNCTOR, void*>& p1,
                      const std::pair<US_MODULE_LISTENER_FUNCTOR, void*>& p2) const
      {
        return p1.second == p2.second &&
            p1.first.target<void(const US_PREPEND_NAMESPACE(ModuleEvent)&)>() == p2.first.target<void(const US_PREPEND_NAMESPACE(ModuleEvent)&)>();
      }
    };
  US_END_NAMESPACE

  US_BEGIN_NAMESPACE
    template<class X>
    US_SERVICE_LISTENER_FUNCTOR ServiceListenerMemberFunctor(X* x, void (X::*memFn)(const US_PREPEND_NAMESPACE(ServiceEvent)))
    { return std::bind1st(std::mem_fun(memFn), x); }
  US_END_NAMESPACE

  US_BEGIN_NAMESPACE
    struct ServiceListenerCompare : std::binary_function<US_SERVICE_LISTENER_FUNCTOR, US_SERVICE_LISTENER_FUNCTOR, bool>
    {
      bool operator()(const US_SERVICE_LISTENER_FUNCTOR& f1,
                      const US_SERVICE_LISTENER_FUNCTOR& f2) const
      {
        return f1.target<void(const US_PREPEND_NAMESPACE(ServiceEvent)&)>() == f2.target<void(const US_PREPEND_NAMESPACE(ServiceEvent)&)>();
      }
    };
  US_END_NAMESPACE

  US_HASH_FUNCTION_NAMESPACE_BEGIN
  US_HASH_FUNCTION_BEGIN(US_SERVICE_LISTENER_FUNCTOR)
    void(*targetFunc)(const US_PREPEND_NAMESPACE(ServiceEvent)&) = arg.target<void(const US_PREPEND_NAMESPACE(ServiceEvent)&)>();
    void* targetPtr = NULL;
    std::memcpy(&targetPtr, &targetFunc, sizeof(void*));
    return US_HASH_FUNCTION(void*, targetPtr);
  US_HASH_FUNCTION_END
  US_HASH_FUNCTION_NAMESPACE_END

#else

  #include <usFunctor_p.h>

  #define US_MODULE_LISTENER_FUNCTOR US_PREPEND_NAMESPACE(Functor)<const US_PREPEND_NAMESPACE(ModuleEvent)&>

  US_BEGIN_NAMESPACE
    template<class X, typename MemFn>
    US_MODULE_LISTENER_FUNCTOR ModuleListenerMemberFunctor(X* x, MemFn memFn)
    { return Functor<const ModuleEvent&>(x, memFn); }
  US_END_NAMESPACE

  US_BEGIN_NAMESPACE
    struct ModuleListenerCompare : std::binary_function<std::pair<US_MODULE_LISTENER_FUNCTOR, void*>,
                                                        std::pair<US_MODULE_LISTENER_FUNCTOR, void*>, bool>
    {
      bool operator()(const std::pair<US_MODULE_LISTENER_FUNCTOR, void*>& p1,
                      const std::pair<US_MODULE_LISTENER_FUNCTOR, void*>& p2) const
      { return p1.second == p2.second && p1.first == p2.first; }
    };
  US_END_NAMESPACE

  #define US_SERVICE_LISTENER_FUNCTOR US_PREPEND_NAMESPACE(Functor)<const US_PREPEND_NAMESPACE(ServiceEvent)&>

  US_BEGIN_NAMESPACE
    template<class X, typename MemFn>
    US_SERVICE_LISTENER_FUNCTOR ServiceListenerMemberFunctor(X* x, MemFn memFn)
    { return Functor<const ServiceEvent&>(x, memFn); }
  US_END_NAMESPACE

  US_BEGIN_NAMESPACE
    struct ServiceListenerCompare : std::binary_function<US_SERVICE_LISTENER_FUNCTOR, US_SERVICE_LISTENER_FUNCTOR, bool>
    {
      bool operator()(const US_SERVICE_LISTENER_FUNCTOR& f1,
                      const US_SERVICE_LISTENER_FUNCTOR& f2) const
      { return f1 == f2; }
    };
  US_END_NAMESPACE

  US_HASH_FUNCTION_NAMESPACE_BEGIN
  US_HASH_FUNCTION_BEGIN(US_SERVICE_LISTENER_FUNCTOR)
    return US_HASH_FUNCTION(void*, reinterpret_cast<void*>(arg.target()));
  US_HASH_FUNCTION_END
  US_HASH_FUNCTION_NAMESPACE_END

#endif

#endif // USLISTENERFUNCTORS_P_H
