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

#ifndef CPPMICROSERVICES_TESTUTILBUNDLELISTENER_H
#define CPPMICROSERVICES_TESTUTILBUNDLELISTENER_H

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/ServiceEvent.h"

namespace cppmicroservices
{

    class TestBundleListener
    {

      public:
        TestBundleListener();

        void BundleChanged(BundleEvent const& event);

        void ServiceChanged(ServiceEvent const& event);

        BundleEvent GetBundleEvent() const;

        ServiceEvent GetServiceEvent() const;

        bool CheckListenerEvents(bool pexp,
                                 BundleEvent::Type ptype,
                                 bool sexp,
                                 ServiceEvent::Type stype,
                                 Bundle const& bundleX,
                                 ServiceReferenceU* servX);

        bool CheckListenerEvents(std::vector<BundleEvent> const& pEvts, bool relaxed = false);

        bool CheckListenerEvents(std::vector<ServiceEvent> const& seEvts);

        bool CheckListenerEvents(std::vector<BundleEvent> const& pEvts,
                                 std::vector<ServiceEvent> const& seEvts,
                                 bool relaxed = false);

      private:
        std::vector<ServiceEvent> serviceEvents;
        std::vector<BundleEvent> bundleEvents;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_TESTUTILBUNDLELISTENER_H
