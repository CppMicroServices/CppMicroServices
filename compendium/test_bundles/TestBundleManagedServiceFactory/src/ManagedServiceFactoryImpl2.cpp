#include "ManagedServiceFactoryImpl2.hpp"

#include <iostream>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryImpl2::~TestManagedServiceFactoryImpl2() = default;

                void
                TestManagedServiceFactoryImpl2::Activate(
                    std::shared_ptr<cppmicroservices::service::component::ComponentContext> const& context)
                {
                    bundleContext_ = context->GetBundleContext();
                }

                void
                TestManagedServiceFactoryImpl2::Updated(std::string const& pid, AnyMap const& properties)
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
                TestManagedServiceFactoryImpl2::Removed(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_removedMtx);
                    ++m_removedCallCount[pid];
                }

                int
                TestManagedServiceFactoryImpl2::getUpdatedCounter(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_updatedMtx);
                    return m_updatedCallCount[pid];
                }

                int
                TestManagedServiceFactoryImpl2::getRemovedCounter(std::string const& pid)
                {
                    std::lock_guard<std::mutex> lk(m_removedMtx);
                    return m_removedCallCount[pid];
                }

                std::shared_ptr<::test::TestManagedServiceFactoryServiceInterface>
                TestManagedServiceFactoryImpl2::create(std::string const& config)
                {

                    std::lock_guard<std::mutex> lk(m_updatedMtx);
                    try
                    {
                        return std::make_shared<TestManagedServiceFactoryServiceImpl2>(m_updatedCallCount.at(config));
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
