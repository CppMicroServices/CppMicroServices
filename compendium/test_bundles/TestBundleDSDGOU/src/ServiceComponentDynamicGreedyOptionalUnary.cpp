#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentDynamicGreedyOptionalUnary::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponentDynamicGreedyOptionalUnary::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponentDynamicGreedyOptionalUnary::ExtendedDescription()
    {
        std::string result("ServiceComponentDynamicGreedyOptionalUnary ");
        result.append("depends on ");
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo)
        {
            result.append(foo->Description());
        }
        return result;
    }

    void
    ServiceComponentDynamicGreedyOptionalUnary::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentDynamicGreedyOptionalUnary::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
