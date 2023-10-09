#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    cppmicroservices::AnyMap
    ServiceComponentBV1_5::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
