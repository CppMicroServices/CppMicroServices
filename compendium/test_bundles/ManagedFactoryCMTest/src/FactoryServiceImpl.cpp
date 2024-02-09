#include "FactoryServiceImpl.hpp"

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                TestManagedFactoryServiceImpl::TestManagedFactoryServiceImpl(int initialValue) : val(initialValue) {}

                TestManagedFactoryServiceImpl::~TestManagedFactoryServiceImpl() = default;

                int
                TestManagedFactoryServiceImpl::getValue()
                {
                    return val;
                }

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
