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

#ifndef CPPMICROSERVICES_LISTENERFUNCTORS_H
#define CPPMICROSERVICES_LISTENERFUNCTORS_H

#include "cppmicroservices/GlobalConfig.h"

#include <cstring>
#include <functional>
#include <cstdint>

namespace cppmicroservices {

  class ServiceEvent;
  class BundleEvent;
  class FrameworkEvent;
  class ServiceListeners;

  using ListenerTokenId = std::uint64_t;

  /**
   * \brief The token returned when a listener is registered with BundleContext.
   *
   * The token object enables the clients to remove the listeners from the BundleContext.
   * This is a move-only type, with the intention that the transfer of ownership will
   * be handled explicitly by the clients.
   *
   */
  class ListenerToken
  {
  public:

    /**
     * Constructs a default, invalid %ListenerToken object.
     * Since, this is not associated with any valid listener, a RemoveListener
     * call taking a default ListenerToken object will do nothing.
     */
    ListenerToken() : tokenId(ListenerTokenId(0)) {};

    ListenerToken(const ListenerToken&) = delete;

    ListenerToken& operator=(const ListenerToken&) = delete;

    ListenerToken(ListenerToken&& other) : tokenId(std::move(other.tokenId))
    {
      other.tokenId = ListenerTokenId(0);
    }

    ListenerToken& operator=(ListenerToken&& other)
    {
      if (this != &other)
      {
        tokenId = std::move(other.tokenId);
        other.tokenId = ListenerTokenId(0);
      }
      return *this;
    }

  private:
    // For internal use
    friend class ServiceListeners;

    ListenerToken(ListenerTokenId _tokenId) : tokenId(_tokenId) {}

    ListenerTokenId getId() const
    {
      return tokenId;
    }

    ListenerTokenId tokenId;
  };

  /**
  \defgroup gr_listeners Listeners

  \brief Groups Listener related symbols.
  */

  /**
   * \ingroup MicroServices
   * \ingroup gr_listeners
   *
   * A \c ServiceEvent listener.
   *
   * A \c ServiceListener can be any callable object and is registered
   * with the Framework using the
   * {@link BundleContext#AddServiceListener(const ServiceListener&, const std::string&)} method.
   * \c ServiceListener instances are called with a \c ServiceEvent object when a
   * service has been registered, unregistered, or modified.
   *
   * @see ServiceEvent
   */
  typedef std::function<void(const ServiceEvent&)> ServiceListener;

  /**
   * \ingroup MicroServices
   * \ingroup gr_listeners
   *
   * A \c BundleEvent listener. When a \c BundleEvent is fired, it is
   * asynchronously (if threading support is enabled) delivered to a
   * \c BundleListener. The Framework delivers \c BundleEvent objects to
   * a \c BundleListener in order and does not concurrently call a
   * \c BundleListener.
   *
   * A \c BundleListener can be any callable object and is registered
   * with the Framework using the
   * {@link BundleContext#AddBundleListener(const BundleListener&)} method.
   * \c BundleListener instances are called with a \c BundleEvent object when a
   * bundle has been installed, resolved, started, stopped, updated, unresolved,
   * or uninstalled.
   *
   * @see BundleEvent
   */
  typedef std::function<void(const BundleEvent&)> BundleListener;

  /**
   * \ingroup MicroServices
   * \ingroup gr_listeners
   *
   * A \c FrameworkEvent listener. When a \c BundleEvent is fired, it is
   * asynchronously (if threading support is enabled) delivered to a
   * \c FrameworkListener. The Framework delivers \c FrameworkEvent objects to
   * a \c FrameworkListener in order and does not concurrently call a
   * \c FrameworkListener.
   *
   * A \c FrameworkListener can be any callable object and is registered
   * with the Framework using the
   * {@link BundleContext#AddFrameworkListener(const FrameworkListener&)} method.
   * \c FrameworkListener instances are called with a \c FrameworkEvent object when a
   * framework life-cycle event or notification message occured.
   *
   * @see FrameworkEvent
   */
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

#endif // CPPMICROSERVICES_LISTENERFUNCTORS_H
