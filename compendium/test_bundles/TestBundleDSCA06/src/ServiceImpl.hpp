#ifndef SERVICE_IMPL_DSCA06_HPP
#define SERVICE_IMPL_DSCA06_HPP

#include "TestInterfaces/Interfaces.hpp"
#include <cppmicroservices/cm/ManagedService.hpp>

namespace sample
{
    class ServiceComponentCA06
        : public ::test::TestManagedServiceInterface
        , public cppmicroservices::service::cm::ManagedService
    {
      public:
        ServiceComponentCA06() = default;
        void Updated(cppmicroservices::AnyMap const& properties) override;
        ~ServiceComponentCA06() = default;
        int
        getCounter() override
        {
            return 1;
        }
    };
} // namespace sample

#endif // SERVICE_IMPL_DSCA06_HPP
