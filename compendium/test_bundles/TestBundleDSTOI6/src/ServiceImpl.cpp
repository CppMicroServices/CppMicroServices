#include "ServiceImpl.hpp"

namespace sample
{
    void
    ServiceComponent6::Activate(std::shared_ptr<ComponentContext> const& ctxt)
    {
        foo = ctxt->LocateService<test::Interface1>("foo");
    }

    void
    ServiceComponent6::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
        foo = nullptr;
    }

    std::string
    ServiceComponent6::ExtendedDescription()
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
    ServiceComponent6::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent6::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
