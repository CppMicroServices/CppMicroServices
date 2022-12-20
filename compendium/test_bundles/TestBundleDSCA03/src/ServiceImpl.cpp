#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    cppmicroservices::AnyMap
    ServiceComponentCA03::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
