#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include "TestInterfaces/Interfaces.hpp"
#include "cppmicroservices/ServiceReference.h"
#include <future>
#include <iostream>
#include <thread>

class dependentImpl : public test::FactoryServiceDependent
{
  public:
    dependentImpl() = default;
    ~dependentImpl() override
    {
        if (_worker.joinable())
        {
            _worker.join();
        }
    }

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

        _factoryCreated = factoryCreated;
        dispatchAsyncWork();
    }

    void UnbindcreatedSvc(std::shared_ptr<test::FactoryCreatedService> /**/) {};

    bool
    didBind() override
    {
        return _bound;
    }

    std::string
    getAsyncSayHiResult() override
    {
        if (_asyncResult.valid())
        {
            return _asyncResult.get();
        }
        return "";
    }

  private:
    void
    dispatchAsyncWork()
    {
        if (!_factoryCreated || _asyncResult.valid())
        {
            return;
        }
        auto promise = std::make_shared<std::promise<std::string>>();
        _asyncResult = promise->get_future();
        _worker = std::thread(
            [promise, svc = _factoryCreated]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                try
                {
                    promise->set_value(svc->sayHi());
                }
                catch (...)
                {
                    promise->set_exception(std::current_exception());
                }
            });
    }

    bool _bound = false;
    std::shared_ptr<test::FactoryCreatedService> _factoryCreated;
    std::future<std::string> _asyncResult;
    std::thread _worker;
};

#endif // _SERVICE_IMPL_HPP_
