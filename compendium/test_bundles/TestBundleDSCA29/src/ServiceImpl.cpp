#include "ServiceImpl.hpp"

namespace sample
{

    std::atomic<int> ServiceComponentCA29::constructionCount { 0 };
    std::atomic<int> ServiceComponentCA29::missingPropertyCount { 0 };

    ServiceComponentCA29::ServiceComponentCA29(std::shared_ptr<cppmicroservices::AnyMap> const& props)
        : properties(props)
    {
        ++constructionCount;
        if (props->find("testProperty") == props->end())
        {
            ++missingPropertyCount;
        }
    }

    cppmicroservices::AnyMap
    ServiceComponentCA29::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
