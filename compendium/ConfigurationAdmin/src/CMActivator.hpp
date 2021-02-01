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

#ifndef CMACTIVATOR_HPP
#define CMACTIVATOR_HPP

#include <memory>
#include <mutex>
#include <unordered_map>

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/ListenerToken.h"

#include "CMBundleExtension.hpp"
#include "CMConstants.hpp"
#include "CMLogger.hpp"
#include "ConfigurationAdminImpl.hpp"

namespace cppmicroservices {
  namespace cmimpl {
    class CMActivator final : public cppmicroservices::BundleActivator
    {
    public:
      CMActivator() = default;
      CMActivator(const CMActivator&) = delete;
      CMActivator(CMActivator&&) = delete;
      CMActivator& operator=(const CMActivator&) = delete;
      CMActivator& operator=(CMActivator&&) = delete;
      ~CMActivator() override = default;

      // callback methods for bundle lifecycle
      void Start(cppmicroservices::BundleContext context) override;
      void Stop(cppmicroservices::BundleContext context) override;

    // protected for pkgtests
    protected:
      /**
       * Bundle listener callback
       */
      void BundleChanged(const cppmicroservices::BundleEvent&);
      /*
       * This method creates the CMBundleExtension object for a bundle
       * with cm configuration metadata
       */
      void CreateExtension(const cppmicroservices::Bundle& bundle);
      /*
       * This method removes the CMBundleExtension object for a bundle
       * with cm configuration metadata
       */
      void RemoveExtension(const cppmicroservices::Bundle& bundle);
    private:
      cppmicroservices::BundleContext runtimeContext;
      std::shared_ptr<CMLogger> logger;
      std::shared_ptr<ConfigurationAdminImpl> configAdminImpl;
      std::mutex bundleRegMutex;
      std::unordered_map<long, std::unique_ptr<CMBundleExtension>> bundleRegistry;
      cppmicroservices::ListenerToken bundleListenerToken;
      cppmicroservices::ServiceRegistration<cppmicroservices::service::cm::ConfigurationAdmin> configAdminReg;
    };
  } // cmimpl
} // cppmicroservices

#endif // CMACTIVATOR_HPP
