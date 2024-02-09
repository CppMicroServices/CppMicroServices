#include "TestInterfaces/Interfaces.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedFactoryServiceImpl : public ::test::TestManagedServiceFactoryServiceInterface
                {
                  public:
                    TestManagedFactoryServiceImpl(int initialValue);
                    ~TestManagedFactoryServiceImpl();
                    int getValue() override;

                  private:
                    int val;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
