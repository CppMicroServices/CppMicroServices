#include "TestInterfaces/Interfaces.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceFactoryServiceImpl : public ::test::TestManagedServiceFactoryServiceInterface
                {
                  public:
                    TestManagedServiceFactoryServiceImpl(int initialValue);
                    ~TestManagedServiceFactoryServiceImpl();

                    int getValue() override;

                  private:
                    int value;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
