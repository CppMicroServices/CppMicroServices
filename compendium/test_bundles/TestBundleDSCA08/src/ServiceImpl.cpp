#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    ServiceComponentCA08::ServiceComponentCA08(std::shared_ptr<cppmicroservices::AnyMap> const& props)
        : properties(props)
    {
    }
    void
    ServiceComponentCA08::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;
    }
    cppmicroservices::AnyMap
    ServiceComponentCA08::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
