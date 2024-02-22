#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    ServiceComponentCA09::ServiceComponentCA09(std::shared_ptr<cppmicroservices::AnyMap> const& props,
                                               const std::shared_ptr<test::Interface1> interface1)
        : properties(props)
        , constructorHit { false }
    {
        if (nullptr != interface1)
        {
            constructorHit = true;
        }
    }
    void
    ServiceComponentCA09::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;
    }
    cppmicroservices::AnyMap
    ServiceComponentCA09::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }
    bool
    ServiceComponentCA09::isDependencyInjected()
    {
        return constructorHit;
    }

} // namespace sample
