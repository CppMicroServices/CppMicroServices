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

#ifndef CPPMICROSERVICES_TESTUTILLISTENERHELPERS_H
#define CPPMICROSERVICES_TESTUTILLISTENERHELPERS_H

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/ServiceEvent.h"
#include "gtest/gtest.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

namespace cppmicroservices
{

    template <class Receiver>
    class BundleListenerRegistrationHelper
    {

      public:
        typedef void (Receiver::*CallbackType)(BundleEvent const&);

        BundleListenerRegistrationHelper(BundleContext const& context, Receiver* receiver, CallbackType callback)
            : context(context)
            , receiver(receiver)
            , callback(callback)
        {
            EXPECT_NO_THROW(this->context.AddBundleListener(receiver, callback))
                << "bundle listener registration failed";
        }

        ~BundleListenerRegistrationHelper() { context.RemoveBundleListener(receiver, callback); }

      private:
        BundleContext context;
        Receiver* receiver;
        CallbackType callback;
    };

    template <class Receiver>
    class ServiceListenerRegistrationHelper
    {

      public:
        typedef void (Receiver::*CallbackType)(ServiceEvent const&);

        ServiceListenerRegistrationHelper(BundleContext const& context, Receiver* receiver, CallbackType callback)
            : context(context)
            , receiver(receiver)
            , callback(callback)
        {
            EXPECT_NO_THROW(this->context.AddServiceListener(receiver, callback))
                << "service listener registration failed";
        }

        ~ServiceListenerRegistrationHelper() { context.RemoveServiceListener(receiver, callback); }

      private:
        BundleContext context;
        Receiver* receiver;
        CallbackType callback;
    };
} // namespace cppmicroservices

US_MSVC_POP_WARNING

#endif // CPPMICROSERVICES_TESTUTILBUNDLELISTENER_H
