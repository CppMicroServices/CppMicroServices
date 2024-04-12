#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    ServiceAImpl::ServiceAImpl(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                                               const std::shared_ptr<test::ServiceBInt> interface1)
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
  
} // namespace sample
