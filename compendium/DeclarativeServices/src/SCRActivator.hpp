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

#ifndef SCRACTIVATOR_HPP
#define SCRACTIVATOR_HPP
#include "ComponentRegistry.hpp"
#include "SCRAsyncWorkService.hpp"
#include "SCRBundleExtension.hpp"
#include "SCRLogger.hpp"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleTracker.h"
#include "cppmicroservices/BundleTrackerCustomizer.h"
#include "cppmicroservices/cm/ConfigurationListener.hpp"
#include "cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp"
#include "manager/ConfigurationNotifier.hpp"
#include <map>
#include <vector>

using cppmicroservices::service::component::runtime::ServiceComponentRuntime;

namespace cppmicroservices {
namespace scrimpl {

class SCRActivator : public cppmicroservices::BundleActivator
{
public:
  SCRActivator() = default;
  SCRActivator(const SCRActivator&) = delete;
  SCRActivator(SCRActivator&&) = delete;
  SCRActivator& operator=(const SCRActivator&) = delete;
  SCRActivator& operator=(SCRActivator&&) = delete;
  ~SCRActivator() override = default;

  class ActivatorCustomizer : public cppmicroservices::BundleTrackerCustomizer<>
  {
  public:
    // Constructor
    ActivatorCustomizer(SCRActivator* q_ptr)
      : q_ptr(q_ptr)
    {
    }

    // Customizer methods
    std::optional<Bundle> AddingBundle(const Bundle&,
                                       const BundleEvent&) override;
    void ModifiedBundle(const Bundle&, const BundleEvent&, Bundle) override;
    void RemovedBundle(const Bundle&, const BundleEvent&, Bundle) override;

  private:
    SCRActivator* q_ptr;
  };

  // callback methods for bundle lifecycle
  void Start(cppmicroservices::BundleContext context) override;
  void Stop(cppmicroservices::BundleContext context) override;

protected:
  /*
   * This method creates the BundleExtension object for a bundle
   * with declarative services metadata
   */
  void CreateExtension(const cppmicroservices::Bundle& bundle);
  /*
   * This method removes the BundleExtension object for a bundle
   * with declarative services metadata
   */
  void DisposeExtension(const cppmicroservices::Bundle& bundle);

private:
  cppmicroservices::BundleContext runtimeContext;
  cppmicroservices::ServiceRegistration<ServiceComponentRuntime> scrServiceReg;
  std::shared_ptr<ComponentRegistry> componentRegistry;
  std::mutex bundleRegMutex;
  std::unordered_map<long, std::unique_ptr<SCRBundleExtension>> bundleRegistry;
  std::shared_ptr<SCRLogger> logger;
  // ListenerToken bundleListenerToken;
  std::shared_ptr<SCRAsyncWorkService> asyncWorkService;
  cppmicroservices::ServiceRegistration<
    cppmicroservices::service::cm::ConfigurationListener>
    configListenerReg;
  std::shared_ptr<ConfigurationNotifier> configNotifier;
  std::shared_ptr<cppmicroservices::BundleTracker<>> bundleTracker;
  std::shared_ptr<ActivatorCustomizer> customizer;
};
} // scrimpl
} // cppmicroservices
#endif // SCRACTIVATOR_HPP
