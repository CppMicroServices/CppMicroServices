#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    ServiceAImpl2::ServiceAImpl2(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                                 const std::shared_ptr<test::ServiceBInt> interface1,
                                 const std::shared_ptr<test::ServiceCInt> interface2)
    : properties(*props)
    , serviceB(interface1)
    , serviceC(interface2)
    {
    }
    void
    ServiceAImpl2::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                            std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = *configuration;
    }
    cppmicroservices::AnyMap
    ServiceAImpl2::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return properties;
    }
  
} // namespace sample
