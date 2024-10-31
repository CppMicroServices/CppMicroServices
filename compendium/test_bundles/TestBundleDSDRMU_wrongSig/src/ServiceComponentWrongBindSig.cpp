#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    std::string
    ServiceComponentWrongBindSig::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result("ServiceComponentWrongBindSig ");
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    void
    ServiceComponentWrongBindSig::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentWrongBindSig::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }
    void
    ServiceComponentWrongBindSig::Modified(std::shared_ptr<ComponentContext> const& /*context*/,
                                           std::shared_ptr<cppmicroservices::AnyMap> const& /*configuration*/)
    {
    }
    void
    ServiceComponentWrongBindSig::Activate(std::shared_ptr<ComponentContext> const& /*context*/)
    {
    }
    void
    ServiceComponentWrongBindSig::Deactivate(std::shared_ptr<ComponentContext> const& /*context*/)
    {
    }
} // namespace sample
