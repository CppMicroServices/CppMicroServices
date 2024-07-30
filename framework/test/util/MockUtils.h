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

#ifndef CPPMICROSERVICES_MOCKUTILS_H
#define CPPMICROSERVICES_MOCKUTILS_H

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceEvent.h"

#include "FrameworkPrivate.h"
#include "CoreBundleContext.h"
#include "BundleStorage.h"
#include "BundleContextPrivate.h"
#include "../gtest/Mocks.h"

namespace cppmicroservices
{

    struct CoreBundleContextHolder;

    /*
     * A helper class for writing tests with mocks. Instantiates a concrete
     * Framework, but mocks any filesystem accesses and makes underlying
     * private classes accessible.
     */
    class MockedEnvironment
    {
      public:
        MockedEnvironment(MockBundleStorageMemory* bundleStorage);
        std::vector<Bundle> Install(std::string& location,
                                    cppmicroservices::AnyMap& bundleManifest,
                                    std::shared_ptr<MockBundleResourceContainer> const& resCont);

        Framework framework;
        CoreBundleContext* coreBundleContext;
        BundleRegistry* bundleRegistry;
        BundlePrivate* bundlePrivate;
        BundleContext bundleContext;
        BundleContextPrivate* bundleContextPrivate;
    };

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_MOCKUTILS_H
