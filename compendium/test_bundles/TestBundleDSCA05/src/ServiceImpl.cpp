#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentCA05::Modified(std::shared_ptr<ComponentContext> const&,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;
    }
    cppmicroservices::AnyMap
    ServiceComponentCA05::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
