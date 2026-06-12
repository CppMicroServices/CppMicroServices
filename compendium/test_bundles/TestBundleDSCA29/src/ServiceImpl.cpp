#include "ServiceImpl.hpp"

namespace sample
{

    ServiceComponentCA29::ServiceComponentCA29(std::shared_ptr<cppmicroservices::AnyMap> const& props)
        : properties(props)
    {
    }

    cppmicroservices::AnyMap
    ServiceComponentCA29::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
