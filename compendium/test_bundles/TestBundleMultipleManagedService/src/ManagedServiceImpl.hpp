#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/cm/ManagedService.hpp>

#include "TestInterfaces/Interfaces.hpp"

#include <mutex>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceImpl3
                    : public ::test::TestManagedServiceInterface
                    , public cppmicroservices::service::cm::ManagedService
                {
                  public:
                    TestManagedServiceImpl3();

                    virtual ~TestManagedServiceImpl3();

                    void Updated(AnyMap const& properties) override;

                    int getCounter() override;

                  private:
                    int m_counter;
                    std::mutex m_counterMtx;
                };

                class TestManagedServiceImpl4
                    : public ::test::TestManagedServiceInterface
                    , public cppmicroservices::service::cm::ManagedService
                {
                  public:
                    TestManagedServiceImpl4();

                    virtual ~TestManagedServiceImpl4();

                    void Updated(AnyMap const& properties) override;

                    int getCounter() override;

                  private:
                    int m_counter;
                    std::mutex m_counterMtx;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
