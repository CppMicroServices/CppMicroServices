#include "ManagedServiceFactoryServiceImpl.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedServiceFactoryServiceImpl::TestManagedServiceFactoryServiceImpl(int initialValue)
                    : value { initialValue }
                {
                }

                TestManagedServiceFactoryServiceImpl::~TestManagedServiceFactoryServiceImpl() = default;

                int
                TestManagedServiceFactoryServiceImpl::getValue()
                {
                    return value;
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
