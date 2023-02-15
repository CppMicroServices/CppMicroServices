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
#include "ManagedServiceImpl.hpp"

#include <assert.h>
#include <iostream>
#include <string>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceImpl::TestManagedServiceImpl() : m_counter { 0 } {}

                TestManagedServiceImpl::~TestManagedServiceImpl() = default;

                void
                TestManagedServiceImpl::Updated(AnyMap const& properties)
                {
                    std::lock_guard<std::mutex> lk(m_counterMtx);
                    if (properties.empty())
                    {
                        // Usually corresponds to the configuration being removed
                        --m_counter;
                    }
                    else
                    {
                        ++m_counter;
                    }
                }

                void
                TestManagedServiceImpl::Activate(
                    std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& ctx)
                {
                    auto installedBundles = ctx->GetBundleContext().GetBundles();
                    auto testBundleIter
                        = std::find_if(installedBundles.begin(),
                                       installedBundles.end(),
                                       [](cppmicroservices::Bundle const& b)
                                       { return (b.GetSymbolicName() == "ManagedServiceAndFactoryBundle"); });
                    assert(testBundleIter != installedBundles.end());
                    testBundleIter->Start();
                }

                int
                TestManagedServiceImpl::getCounter()
                {
                    std::lock_guard<std::mutex> lk(m_counterMtx);
                    return m_counter;
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
