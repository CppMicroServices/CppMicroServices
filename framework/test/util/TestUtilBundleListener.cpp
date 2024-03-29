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

#include "TestUtilBundleListener.h"

#include "cppmicroservices/Bundle.h"

namespace cppmicroservices
{

    TestBundleListener::TestBundleListener() : serviceEvents(), bundleEvents() {}

    void
    TestBundleListener::BundleChanged(BundleEvent const& event)
    {
        bundleEvents.push_back(event);
    }

    void
    TestBundleListener::ServiceChanged(ServiceEvent const& event)
    {
        serviceEvents.push_back(event);
    }

    BundleEvent
    TestBundleListener::GetBundleEvent() const
    {
        if (bundleEvents.empty())
        {
            return BundleEvent();
        }
        return bundleEvents.back();
    }

    ServiceEvent
    TestBundleListener::GetServiceEvent() const
    {
        if (serviceEvents.empty())
        {
            return ServiceEvent();
        }
        return serviceEvents.back();
    }

    bool
    TestBundleListener::CheckListenerEvents(bool pexp,
                                            BundleEvent::Type ptype,
                                            bool sexp,
                                            ServiceEvent::Type stype,
                                            Bundle const& bundleX,
                                            ServiceReferenceU* servX)
    {
        std::vector<BundleEvent> pEvts;
        std::vector<ServiceEvent> seEvts;

        if (pexp)
            pEvts.push_back(BundleEvent(ptype, bundleX));
        if (sexp)
            seEvts.push_back(ServiceEvent(stype, *servX));

        return CheckListenerEvents(pEvts, seEvts);
    }

    bool
    TestBundleListener::CheckListenerEvents(std::vector<BundleEvent> const& pEvts, bool relaxed)
    {
        bool listenState = true; // assume everything will work

        if (pEvts.size() != bundleEvents.size() && !relaxed)
        {
            listenState = false;
            std::cerr << "*** Bundle event mismatch: \nexpected " << pEvts.size() << " event(s), found "
                      << bundleEvents.size() << " event(s).";

            const std::size_t max = pEvts.size() > bundleEvents.size() ? pEvts.size() : bundleEvents.size();
            for (std::size_t i = 0; i < max; ++i)
            {
                BundleEvent const& pE = i < pEvts.size() ? pEvts[i] : BundleEvent();
                BundleEvent const& pR = i < bundleEvents.size() ? bundleEvents[i] : BundleEvent();
                std::cerr << "    " << pE << " - " << pR;
            }
        }
        else
        {
            if (relaxed)
            {
                // just check if the expected events are present
                for (auto e : pEvts)
                {
                    if (std::find(bundleEvents.begin(), bundleEvents.end(), e) == bundleEvents.end())
                    {
                        listenState = false;
                        std::cerr << "*** Expected event not found: " << e;
                        break;
                    }
                }
            }
            else
            {
                // check that the expected events match the received events exactly
                for (std::size_t i = 0; i < pEvts.size(); ++i)
                {
                    BundleEvent const& pE = pEvts[i];
                    BundleEvent const& pR = bundleEvents[i];
                    if (pE.GetType() != pR.GetType() || pE.GetBundle() != pR.GetBundle())
                    {
                        listenState = false;
                        std::cerr << "*** Wrong bundle event: " << pR << "\nexpected " << pE;
                    }
                }
            }
        }

        bundleEvents.clear();
        return listenState;
    }

    bool
    TestBundleListener::CheckListenerEvents(std::vector<ServiceEvent> const& seEvts)
    {
        bool listenState = true; // assume everything will work

        if (seEvts.size() != serviceEvents.size())
        {
            listenState = false;
            std::cerr << "*** Service event mismatch: \nexpected " << seEvts.size() << " event(s), found "
                      << serviceEvents.size() << " event(s).";

            const std::size_t max = seEvts.size() > serviceEvents.size() ? seEvts.size() : serviceEvents.size();
            for (std::size_t i = 0; i < max; ++i)
            {
                ServiceEvent const& seE = i < seEvts.size() ? seEvts[i] : ServiceEvent();
                ServiceEvent const& seR = i < serviceEvents.size() ? serviceEvents[i] : ServiceEvent();
                std::cerr << "    " << seE << " - " << seR;
            }
        }
        else
        {
            for (std::size_t i = 0; i < seEvts.size(); ++i)
            {
                ServiceEvent const& seE = seEvts[i];
                ServiceEvent const& seR = serviceEvents[i];
                if (seE.GetType() != seR.GetType() || (!(seE.GetServiceReference() == seR.GetServiceReference())))
                {
                    listenState = false;
                    std::cerr << "*** Wrong service event: " << seR << "\nexpected " << seE;
                }
            }
        }

        serviceEvents.clear();
        return listenState;
    }

    bool
    TestBundleListener::CheckListenerEvents(std::vector<BundleEvent> const& pEvts,
                                            std::vector<ServiceEvent> const& seEvts,
                                            bool relaxed)
    {
        if (!CheckListenerEvents(pEvts, relaxed))
            return false;
        return CheckListenerEvents(seEvts);
    }
} // namespace cppmicroservices
