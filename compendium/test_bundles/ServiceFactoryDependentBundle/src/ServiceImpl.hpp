#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/ServiceReference.h"
#include <iostream>

class dependentImpl : public test::FactoryServiceDependent
{
  public:
    dependentImpl() = default;
    ~dependentImpl() override = default;

    void
    BindcreatedSvc(std::shared_ptr<test::FactoryCreatedService> factoryCreated)
    {
        try
        {
            auto srefFromService1 = cppmicroservices::ServiceReferenceFromService(factoryCreated);
            US_UNUSED(srefFromService1);
            _bound = true;
        }
        catch (std::exception const& e)
        {
            std::cout << "Exception getting ref: " << e.what() << std::endl;
            return;
        }
    }

    void UnbindcreatedSvc(std::shared_ptr<test::FactoryCreatedService> /**/) {};

    bool
    didBind() override
    {
        return _bound;
    }
    private:
      bool _bound = false;
};

#endif // _SERVICE_IMPL_HPP_
