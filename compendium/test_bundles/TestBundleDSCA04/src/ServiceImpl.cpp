#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    cppmicroservices::AnyMap
    ServiceComponentCA04::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
