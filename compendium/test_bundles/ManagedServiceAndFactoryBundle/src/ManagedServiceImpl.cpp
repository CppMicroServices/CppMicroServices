#include "ManagedServiceImpl.hpp"

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
                        m_counter -= 1;
                    }
                    else
                    {
                        auto const incrementBy = cppmicroservices::any_cast<int>(properties.AtCompoundKey("anInt"));
                        m_counter += incrementBy;
                    }
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
