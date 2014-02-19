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

#include <usServiceEvent.h>
#include <usModuleEvent.h>

#include <algorithm>
#include <cstring>

#ifdef US_HAVE_TR1_FUNCTIONAL_H
  #include <tr1/functional>
#elif defined(US_HAVE_FUNCTIONAL_H)
  #include <functional>
#endif

#ifdef US_HAVE_STD_FUNCTION
  #define US_FUNCTION_TYPE std::function
#elif defined(US_HAVE_TR1_FUNCTION)
  #define US_FUNCTION_TYPE std::tr1::function
#endif

#define US_MODULE_LISTENER_FUNCTOR US_FUNCTION_TYPE<void(const US_PREPEND_NAMESPACE(ModuleEvent)&)>
#define US_SERVICE_LISTENER_FUNCTOR US_FUNCTION_TYPE<void(const US_PREPEND_NAMESPACE(ServiceEvent)&)>

US_BEGIN_NAMESPACE
  template<class X>
  US_MODULE_LISTENER_FUNCTOR ModuleListenerMemberFunctor(X* x, void (X::*memFn)(const US_PREPEND_NAMESPACE(ModuleEvent)))
  { return std::bind1st(std::mem_fun(memFn), x); }

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

  template<class X>
  US_SERVICE_LISTENER_FUNCTOR ServiceListenerMemberFunctor(X* x, void (X::*memFn)(const US_PREPEND_NAMESPACE(ServiceEvent)))
  { return std::bind1st(std::mem_fun(memFn), x); }

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

#endif // USLISTENERFUNCTORS_P_H
