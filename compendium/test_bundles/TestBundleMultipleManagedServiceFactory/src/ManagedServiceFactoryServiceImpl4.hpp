#include "TestInterfaces/Interfaces.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceFactoryServiceImpl4 : public ::test::TestManagedServiceFactoryServiceInterface
                {
                  public:
                    TestManagedServiceFactoryServiceImpl4(int initialValue);
                    ~TestManagedServiceFactoryServiceImpl4();

                    int getValue() override;

                  private:
                    int value;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
