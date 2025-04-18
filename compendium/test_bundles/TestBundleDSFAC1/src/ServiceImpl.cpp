#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    ServiceAImpl::ServiceAImpl(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                               std::shared_ptr<test::ServiceBInt> const interface1)
        : properties(*props)
        , serviceB(interface1)
    {
    }
    void
    ServiceAImpl::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                           std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = *configuration;
    }
    cppmicroservices::AnyMap
    ServiceAImpl::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return properties;
    }

    ServiceAImpl2::ServiceAImpl2(std::shared_ptr<cppmicroservices::AnyMap> const&,
                                 std::shared_ptr<test::ServiceBInt> const interface1)
        : serviceB(interface1)
    {
    }
    ServiceAImpl3::ServiceAImpl3(std::shared_ptr<cppmicroservices::AnyMap> const&,
                                 std::shared_ptr<test::ServiceCInt> const interface1)
        : serviceC(interface1)
    {
    }
    ServiceAImpl4::ServiceAImpl4(std::shared_ptr<cppmicroservices::AnyMap> const&,
                                 std::shared_ptr<test::ServiceBInt> const interface1)
        : serviceB(interface1)
    {
    }
    ServiceAImpl5::ServiceAImpl5(std::shared_ptr<cppmicroservices::AnyMap> const&,
                                 std::shared_ptr<test::ServiceBInt> const interface1)
        : serviceB(interface1)
    {
    }

} // namespace sample
