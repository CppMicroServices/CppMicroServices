#include "TestInterfaces/Interfaces.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceFactoryServiceImpl3 : public ::test::TestManagedServiceFactoryServiceInterface
                {
                  public:
                    TestManagedServiceFactoryServiceImpl3(int initialValue);
                    ~TestManagedServiceFactoryServiceImpl3();

                    int getValue() override;

                  private:
                    int value;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
