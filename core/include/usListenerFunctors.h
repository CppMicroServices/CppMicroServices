/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include <functional>
#include <cstring>

#include "usGlobalConfig.h"

namespace us {

  class ServiceEvent;
  class ModuleEvent;

  typedef std::function<void(const ServiceEvent&)> ServiceListener;
  typedef std::function<void(const ModuleEvent&)> ModuleListener;

  template<class X>
  ServiceListener ServiceListenerMemberFunctor(X* x, void (X::*memFn)(const ServiceEvent&))
  { return std::bind(memFn, x, std::placeholders::_1); }

  template<class X>
  ModuleListener ModuleListenerMemberFunctor(X* x, void (X::*memFn)(const ModuleEvent&))
  { return std::bind(memFn, x, std::placeholders::_1); }


}

US_HASH_FUNCTION_BEGIN(us::ServiceListener)
  typedef void(*TargetType)(const us::ServiceEvent&);
  const TargetType* targetFunc = arg.target<TargetType>();
  void* targetPtr = NULL;
  std::memcpy(&targetPtr, &targetFunc, sizeof(void*));
  return hash<void*>()(targetPtr);
US_HASH_FUNCTION_END

#endif // USLISTENERFUNCTORS_P_H
