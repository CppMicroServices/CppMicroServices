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

#include "cppmicroservices/Bundle.h"

#include "TestUtilFrameworkListener.h"
#include "gtest/gtest.h"

#include <stdexcept>

namespace cppmicroservices
{

    TestFrameworkListener::TestFrameworkListener() : _events() {}
    TestFrameworkListener::~TestFrameworkListener() {};

    std::size_t
    TestFrameworkListener::events_received() const
    {
        return _events.size();
    }

    bool
    TestFrameworkListener::CheckEvents(std::vector<FrameworkEvent> const& events)
    {
        bool listenState = true; // assume success
        if (events.size() != _events.size())
        {
            listenState = false;
            ADD_FAILURE() << "*** Framework event mismatch ***\n expected " << events.size() << " event(s)\n found "
                          << _events.size() << " event(s).";

            const std::size_t max = events.size() > _events.size() ? events.size() : _events.size();
            for (std::size_t i = 0; i < max; ++i)
            {
                FrameworkEvent const& pE = i < events.size() ? events[i] : FrameworkEvent();
                FrameworkEvent const& pR = i < _events.size() ? _events[i] : FrameworkEvent();
                std::cout << " - " << pE << " - " << pR;
            }
        }
        else
        {
            for (std::size_t i = 0; i < events.size(); ++i)
            {
                FrameworkEvent const& pE = events[i];
                FrameworkEvent const& pR = _events[i];
                if (pE.GetType() != pR.GetType() || pE.GetBundle() != pR.GetBundle())
                {
                    listenState = false;
                    ADD_FAILURE() << "*** Wrong framework event ***\n found " << pR << "\n expected " << pE;
                }
            }
        }

        _events.clear();
        return listenState;
    }

    void
    TestFrameworkListener::frameworkEvent(FrameworkEvent const& evt)
    {
        _events.push_back(evt);
    }

    void
    TestFrameworkListener::throwOnFrameworkEvent(FrameworkEvent const&)
    {
        throw std::runtime_error("whoopsie!");
    }
} // namespace cppmicroservices
