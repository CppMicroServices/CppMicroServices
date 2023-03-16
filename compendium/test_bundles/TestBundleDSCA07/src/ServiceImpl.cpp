#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentCA07::Modified(std::shared_ptr<ComponentContext> const&,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;
    }
    cppmicroservices::AnyMap
    ServiceComponentCA07::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
