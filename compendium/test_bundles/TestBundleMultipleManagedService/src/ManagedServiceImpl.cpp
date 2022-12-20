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

                TestManagedServiceImpl3::TestManagedServiceImpl3() : m_counter { 0 } {}

                TestManagedServiceImpl3::~TestManagedServiceImpl3() = default;

                void
                TestManagedServiceImpl3::Updated(AnyMap const&)
                {
                    std::lock_guard<std::mutex> lk(m_counterMtx);
                    m_counter++;
                }

                int
                TestManagedServiceImpl3::getCounter()
                {
                    std::lock_guard<std::mutex> lk(m_counterMtx);
                    return m_counter;
                }

                TestManagedServiceImpl4::TestManagedServiceImpl4() : m_counter { 0 } {}

                TestManagedServiceImpl4::~TestManagedServiceImpl4() = default;

                void
                TestManagedServiceImpl4::Updated(AnyMap const&)
                {
                    std::lock_guard<std::mutex> lk(m_counterMtx);
                    m_counter++;
                }

                int
                TestManagedServiceImpl4::getCounter()
                {
                    std::lock_guard<std::mutex> lk(m_counterMtx);
                    return m_counter;
                }
            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
