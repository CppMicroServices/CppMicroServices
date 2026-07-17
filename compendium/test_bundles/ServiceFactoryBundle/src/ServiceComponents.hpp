#ifndef ServiceComponents_hpp
#define ServiceComponents_hpp
#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/ServiceFactory.h"
#include <iostream>

class factoryCreatedImpl : public test::FactoryCreatedService
{
  public:
    factoryCreatedImpl() = default;
    ~factoryCreatedImpl() = default;
    std::string
    sayHi()
    {
        return "HELLO";
    }
};

class testServiceFactory : public ::cppmicroservices::ServiceFactory
{
  public:
    testServiceFactory() {}

    ::cppmicroservices::InterfaceMapConstPtr
    GetService(::cppmicroservices::Bundle const& /*bundle*/,
               ::cppmicroservices::ServiceRegistrationBase const& /*registration*/) override
    {
        auto obj = std::make_shared<factoryCreatedImpl>();
        return ::cppmicroservices::MakeInterfaceMap<test::FactoryCreatedService>(obj);
    }

    void
    UngetService(::cppmicroservices::Bundle const& /*bundle*/,
                 ::cppmicroservices::ServiceRegistrationBase const& /*registration*/,
                 ::cppmicroservices::InterfaceMapConstPtr const& /**/) override
    {
        return;
    }
};

#endif /* ServiceComponents_hpp */
