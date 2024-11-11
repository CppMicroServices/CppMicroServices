#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    std::string
    ServiceComponent_userMethodStaticChecks::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result("ServiceComponent_userMethodStaticChecks ");
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    void
    ServiceComponent_userMethodStaticChecks::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent_userMethodStaticChecks::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }
    void
    ServiceComponent_userMethodStaticChecks::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                                           std::shared_ptr<cppmicroservices::AnyMap> const& /*configuration*/)
    {
    }
    void
    ServiceComponent_userMethodStaticChecks::Activate(std::shared_ptr<ComponentContext> const& /*context*/)
    {
    }
    void
    ServiceComponent_userMethodStaticChecks::Deactivate(std::shared_ptr<ComponentContext> const& /*context*/)
    {
    }
} // namespace sample
