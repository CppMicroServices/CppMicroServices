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
    ServiceComponent_userMethodStaticChecks::Bindfoo(std::shared_ptr<test::DSGraph01> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent_userMethodStaticChecks::Unbindfoo(std::shared_ptr<test::DSGraph01> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }
    void
    ServiceComponent_userMethodStaticChecks::Modified(
        std::shared_ptr<ComponentContext> const& /*context*/,
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

    std::string
    ServiceComponent_userDSMethodStaticChecks::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result("ServiceComponent_userDSMethodStaticChecks ");
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    void
    ServiceComponent_userDSMethodStaticChecks::Bindfoo(std::shared_ptr<test::DSGraph02> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent_userDSMethodStaticChecks::Unbindfoo(std::shared_ptr<test::DSGraph02> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

    void
    ServiceComponent_userDSMethodStaticChecks::Activate(std::shared_ptr<ComponentContext> const& /*context*/)
    {
    }
    void
    ServiceComponent_userDSMethodStaticChecks::Deactivate(std::shared_ptr<ComponentContext> const& /*context*/)
    {
    }

    std::string
    ServiceComponent_userCAMethodStaticChecks::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result("ServiceComponent_userCAMethodStaticChecks ");
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    void
    ServiceComponent_userCAMethodStaticChecks::Bindfoo(std::shared_ptr<test::DSGraph03> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent_userCAMethodStaticChecks::Unbindfoo(std::shared_ptr<test::DSGraph03> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

    void
    ServiceComponent_userCAMethodStaticChecks::Modified(
        std::shared_ptr<ComponentContext> const& /*context*/,
        std::shared_ptr<cppmicroservices::AnyMap> const& /*configuration*/)
    {
    }
} // namespace sample
