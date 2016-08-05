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

#ifndef LISTENERFUNCTORS_P_H
#define LISTENERFUNCTORS_P_H

#include "cppmicroservices/GlobalConfig.h"

#include <cstring>
#include <functional>

namespace cppmicroservices {

  class ServiceEvent;
  class BundleEvent;
  class FrameworkEvent;

  typedef std::function<void(const ServiceEvent&)> ServiceListener;

  /**
   * \ingroup MicroServices
   *
   * A \c BundleEvent listener. When a \c BundleEvent is fired, it is
   * asynchronously (if threading support is enabled) delivered to a
   * \c BundleListener. The Framework delivers \c BundleEvent objects to
   * a \c BundleListener in order and does not concurrently call a
   * \c BundleListener.
   *
   * A \c BundleListener can be any callable object and is registered
   * with the Framework using the
   * {@link BundleContext#AddBundleListener(BundleListener)} method.
   * {\c BundleListener}s are called with a \c BundleEvent object when a
   * bundle has been installed, resolved, started, stopped, updated, unresolved,
   * or uninstalled.
   *
   * @see BundleEvent
   */
  typedef std::function<void(const BundleEvent&)> BundleListener;
  typedef std::function<void(const FrameworkEvent&)> FrameworkListener;

  template<class X>
  ServiceListener ServiceListenerMemberFunctor(X* x, void (X::*memFn)(const ServiceEvent&))
  { return std::bind(memFn, x, std::placeholders::_1); }

  template<class X>
  BundleListener BundleListenerMemberFunctor(X* x, void (X::*memFn)(const BundleEvent&))
  { return std::bind(memFn, x, std::placeholders::_1); }

  template<class X>
  FrameworkListener BindFrameworkListenerToFunctor(X* x, void (X::*Fnc)(const FrameworkEvent&))
  { return std::bind(Fnc, x, std::placeholders::_1); }

}

US_HASH_FUNCTION_BEGIN(cppmicroservices::ServiceListener)
  typedef void(*TargetType)(const cppmicroservices::ServiceEvent&);
  const TargetType* targetFunc = arg.target<TargetType>();
  void* targetPtr = nullptr;
  std::memcpy(&targetPtr, &targetFunc, sizeof(void*));
  return hash<void*>()(targetPtr);
US_HASH_FUNCTION_END

#endif // LISTENERFUNCTORS_P_H
