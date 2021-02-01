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

#ifndef __COMPONENTMANAGERIMPL_HPP__
#define __COMPONENTMANAGERIMPL_HPP__

#include "boost/asio/thread_pool.hpp"
#if defined(USING_GTEST)
#include "gtest/gtest_prod.h"
#else
#define FRIEND_TEST(x, y)
#endif
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "ComponentManager.hpp"

namespace cppmicroservices {
namespace scrimpl {

class ComponentRegistry;
class ComponentManagerState;

/**
 * This class is responsible for managing the enabled/disabled states of a
 * service component. It implements a thread safe state design pattern to
 * handle requests for enabling and disabling a component.
 */
class ComponentManagerImpl
  : public ComponentManager
{
public:
  ComponentManagerImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                       std::shared_ptr<const ComponentRegistry> registry,
                       cppmicroservices::BundleContext bundleContext,
                       std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                       std::shared_ptr<boost::asio::thread_pool> threadpool);
  ComponentManagerImpl(const ComponentManagerImpl&) = delete;
  ComponentManagerImpl(ComponentManagerImpl&&) = delete;
  ComponentManagerImpl& operator=(const ComponentManagerImpl&) = delete;
  ComponentManagerImpl& operator=(ComponentManagerImpl&&) = delete;
  ~ComponentManagerImpl() override;

  /**
   * Initialization method used to kick start the state machine implemented by this class.
   */
  void Initialize();

  /** @copydoc ComponentManager::IsEnabled()
   * Delegates the call to the current state object
   */
  bool IsEnabled() const override;

  /** @copydoc ComponentManager::Enable()
   * Delegates the call to the current state object
   */
  std::shared_future<void> Enable() override;

  /** @copydoc ComponentManager::Disable()
   * Delegates the call to the current state object
   */
  std::shared_future<void> Disable() override;

  /** @copydoc ComponentManager::GetComponentConfigurations()
   * Delegates the call to the current state object
   */
  std::vector<std::shared_ptr<ComponentConfiguration>> GetComponentConfigurations() const override;

  /** @copydoc ComponentManager::GetMetadata()
   * Returns the stored component description
   */
  std::shared_ptr<const metadata::ComponentMetadata> GetMetadata() const override { return compDesc; }

  /** @copydoc ComponentManager::GetName()
   * Returns the names from the stored component description
   */
  std::string GetName() const override { return GetMetadata()->name; }

  /** @copydoc ComponentManager::GetBundleId()
   * Returns the id of the {@link Bundle} which contains the component managed by this object
   */
  unsigned long GetBundleId() const override { return GetBundle().GetBundleId(); }

  /**
   * This method returns the {@link Bundle} which contains the component managed by this object.
   */
  Bundle GetBundle() const { return bundleContext ? bundleContext.GetBundle() : Bundle(); }

  /**
   * Returns the logger object associated with this ComponentManager
   */
  std::shared_ptr<cppmicroservices::logservice::LogService> GetLogger() const
  { return logger; }

  /**
   * This method modifies the vector of futures stored in this object. If
   * any of the futures in the vector are ready, the ready future is replaced
   * by the given future. If none of the futures are ready, the given future
   * is added to the vector.
   */
  void AccumulateFuture(std::shared_future<void> fObj);

  /**
   * Method used to set the state of this object. It invokes the std::atomic
   * operations on the state member of this object.
   *
   * \param expectedState is the pointer to the current state object
   * \param desiredState is the state the caller wishes to set on this object
   */
  virtual bool CompareAndSetState(std::shared_ptr<ComponentManagerState>* expectedState,
                                  std::shared_ptr<ComponentManagerState> desiredState);

  /**
   * This method returns the current state object of this object.
   */
  std::shared_ptr<ComponentManagerState> GetState() const;

  /**
   * Returns the {@link ComponentRegistry} object associated with this
   * runtime instance
   */
  virtual std::shared_ptr<const ComponentRegistry> GetRegistry() const { return registry; }

  /**
   * Attempts to change the state from disabled to enabled and posts asynchronous work
   * to be completed if the state was successfully changed.
   * 
   * \param currentState The current state object
   * \return a shared_future<void> on which to wait for the asynchronous work to complete.
   */
  std::shared_future<void> PostAsyncDisabledToEnabled(
    std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>&
      currentState);

  /**
   * Attempts to change the state from enabled to disabled and posts asynchronous work
   * to be completed if the state was successfully changed.
   * 
   * \param currentState The current state object
   * \return a shared_future<void> on which to wait for the asynchronous work to complete.
   */
  std::shared_future<void> PostAsyncEnabledToDisabled(
    std::shared_ptr<cppmicroservices::scrimpl::ComponentManagerState>&
      currentState);


private:
  FRIEND_TEST(ComponentManagerImplParameterizedTest, TestAccumulateFutures);

  const std::shared_ptr<const ComponentRegistry> registry; ///< component registry associated with the current runtime
  const std::shared_ptr<const metadata::ComponentMetadata> compDesc; ///< the component description
  cppmicroservices::BundleContext bundleContext; ///< context of the bundle which contains the component
  const std::shared_ptr<cppmicroservices::logservice::LogService> logger; ///< logger associated with the current runtime
  std::shared_ptr<ComponentManagerState> state; ///< This member is always accessed using atomic operations
  std::vector<std::shared_future<void>> disableFutures; ///< futures created when the component transitioned to \c DISABLED state
  std::mutex futuresMutex; ///< mutex to protect the #disableFutures member
  std::shared_ptr<boost::asio::thread_pool> threadpool; ///< thread pool used to execute async work
  std::mutex transitionMutex; ///< mutex to make the state transition and posting of the async operations atomic
};
}
}

#endif /* __COMPONENTMANAGERIMPL_HPP__ */
