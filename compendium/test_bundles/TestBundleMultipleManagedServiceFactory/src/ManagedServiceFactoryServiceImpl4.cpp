#include "ManagedServiceFactoryServiceImpl4.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryServiceImpl4::TestManagedServiceFactoryServiceImpl4(int initialValue)
                    : value { initialValue }
                {
                }

                TestManagedServiceFactoryServiceImpl4::~TestManagedServiceFactoryServiceImpl4() = default;

                int
                TestManagedServiceFactoryServiceImpl4::getValue()
                {
                    return value;
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
