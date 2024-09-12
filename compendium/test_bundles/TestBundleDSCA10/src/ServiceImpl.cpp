#include "ServiceImpl.hpp"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"
#include <iostream>

namespace sample
{

    ServiceComponentCA10::ServiceComponentCA10(std::shared_ptr<cppmicroservices::AnyMap> const& props)
        : properties(props)
    {
    }
    void
    ServiceComponentCA10::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;
        auto CA = cppmicroservices::any_cast<std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin>>(
            properties->at("CA"));

        auto configObject = CA->GetConfiguration(cppmicroservices::any_cast<std::string>(properties->at("configID")));

        cppmicroservices::AnyMap newprops(cppmicroservices::AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS);

        newprops["uniqueProp"] = std::string("UNIQUE");
        auto fut = configObject->Update(newprops);
        fut.get();
    }
    cppmicroservices::AnyMap
    ServiceComponentCA10::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
