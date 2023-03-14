#include "ManagedServiceFactoryImpl3.hpp"

#include <iostream>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryImpl3::~TestManagedServiceFactoryImpl3() = default;

                void
                TestManagedServiceFactoryImpl3::Activate(
                    std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context)
                {
                    bundleContext_ = context->GetBundleContext();
                }

                void
                TestManagedServiceFactoryImpl3::Updated(std::string const& pid, AnyMap const&)
                {
                    std::lock_guard<std::mutex> lk(m_updatedMtx);
                    m_updatedCallCount[pid] += 1;
                }

                void
                TestManagedServiceFactoryImpl3::Removed(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_removedMtx);
                    ++m_removedCallCount[pid];
                }

                int
                TestManagedServiceFactoryImpl3::getUpdatedCounter(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_updatedMtx);
                    return m_updatedCallCount[pid];
                }

                int
                TestManagedServiceFactoryImpl3::getRemovedCounter(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_removedMtx);
                    return m_removedCallCount[pid];
                }

                std::shared_ptr<::test::TestManagedServiceFactoryServiceInterface>
                TestManagedServiceFactoryImpl3::create(std::string const& config)
                {

                    std::lock_guard<std::mutex> lk(m_updatedMtx);
                    try
                    {
                        return std::make_shared<TestManagedServiceFactoryServiceImpl3>(m_updatedCallCount.at(config));
                    }
                    catch (...)
                    {
                        return nullptr;
                    }
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
