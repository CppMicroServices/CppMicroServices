#include "ManagedServiceFactoryServiceImpl3.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryServiceImpl3::TestManagedServiceFactoryServiceImpl3(int initialValue)
                    : value { initialValue }
                {
                }

                TestManagedServiceFactoryServiceImpl3::~TestManagedServiceFactoryServiceImpl3() = default;

                int
                TestManagedServiceFactoryServiceImpl3::getValue()
                {
                    return value;
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
