#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentBV1_2::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponentBV1_2::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponentBV1_2::ExtendedDescription()
    {
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result(STRINGIZE(US_BUNDLE_NAME));
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    void
    ServiceComponentBV1_2::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentBV1_2::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
