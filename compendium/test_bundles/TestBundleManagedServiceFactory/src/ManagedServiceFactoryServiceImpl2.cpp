#include "ManagedServiceFactoryServiceImpl2.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryServiceImpl2::TestManagedServiceFactoryServiceImpl2(int initialValue)
                    : value { initialValue }
                {
                }

                TestManagedServiceFactoryServiceImpl2::~TestManagedServiceFactoryServiceImpl2() = default;

                int
                TestManagedServiceFactoryServiceImpl2::getValue()
                {
                    return value;
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
