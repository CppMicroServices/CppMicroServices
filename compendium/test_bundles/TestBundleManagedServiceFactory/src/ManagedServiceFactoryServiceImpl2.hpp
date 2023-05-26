#include "TestInterfaces/Interfaces.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceFactoryServiceImpl2 : public ::test::TestManagedServiceFactoryServiceInterface
                {
                  public:
                    TestManagedServiceFactoryServiceImpl2(int initialValue);
                    ~TestManagedServiceFactoryServiceImpl2();

                    int getValue() override;

                  private:
                    int value;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
