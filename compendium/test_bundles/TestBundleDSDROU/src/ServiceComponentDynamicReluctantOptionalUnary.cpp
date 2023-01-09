#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentDynamicReluctantOptionalUnary::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponentDynamicReluctantOptionalUnary::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponentDynamicReluctantOptionalUnary::ExtendedDescription()
    {
        std::string result("ServiceComponentDynamicReluctantOptionalUnary ");
        result.append("depends on ");
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo)
        {
            result.append(foo->Description());
        }
        return result;
    }

    void
    ServiceComponentDynamicReluctantOptionalUnary::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentDynamicReluctantOptionalUnary::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
