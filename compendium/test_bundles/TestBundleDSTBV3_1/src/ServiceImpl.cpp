#include "ServiceImpl.hpp"

namespace sample
{
    void
    ServiceComponentBV3_1::Activate(std::shared_ptr<ComponentContext> const& ctxt)
    {
        foo = ctxt->LocateService<test::Interface1>("foo");
    }

    void
    ServiceComponentBV3_1::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
        foo = nullptr;
    }

    std::string
    ServiceComponentBV3_1::ExtendedDescription()
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
    ServiceComponentBV3_1::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentBV3_1::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
