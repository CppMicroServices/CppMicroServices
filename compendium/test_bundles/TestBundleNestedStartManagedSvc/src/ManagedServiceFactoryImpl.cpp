#include "ManagedServiceFactoryImpl.hpp"

#include <assert.h>
#include <iostream>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryImpl::~TestManagedServiceFactoryImpl() = default;

                void
                TestManagedServiceFactoryImpl::Activate(
                    std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context)
                {
                    auto installedBundles = context->GetBundleContext().GetBundles();
                    auto testBundleIter
                        = std::find_if(installedBundles.begin(),
                                       installedBundles.end(),
                                       [](cppmicroservices::Bundle const& b)
                                       { return (b.GetSymbolicName() == "ManagedServiceAndFactoryBundle"); });
                    assert(testBundleIter != installedBundles.end());
                    testBundleIter->Start();
                }

                void
                TestManagedServiceFactoryImpl::Updated(std::string const& pid, AnyMap const& properties)
                {
                    std::lock_guard<std::mutex> lk(m_updatedMtx);
                    if (properties.empty())
                    {
                        m_updatedCallCount[pid] -= 1;
                    }
                    else
                    {
                        m_updatedCallCount[pid] += 1;
                    }
                }

                void
                TestManagedServiceFactoryImpl::Removed(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_removedMtx);
                    ++m_removedCallCount[pid];
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
