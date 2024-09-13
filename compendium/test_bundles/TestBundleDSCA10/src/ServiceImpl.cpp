#include "ServiceImpl.hpp"
#include "cppmicroservices/Bundle.h"
#include <iostream>

namespace sample
{

    ServiceComponentCA10::ServiceComponentCA10(
        std::shared_ptr<cppmicroservices::AnyMap> const& props,
        std::shared_ptr<cppmicroservices::service::cm::ConfigurationAdmin> const& CA)
        : properties(props)
        , CA(CA)
    {
    }
    void
    ServiceComponentCA10::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;

        auto configObject = CA->GetConfiguration(cppmicroservices::any_cast<std::string>(properties->at("configID")));

        auto newprops = configObject->GetProperties();

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
