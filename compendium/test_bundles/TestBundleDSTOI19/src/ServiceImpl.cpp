#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    std::string
    ServiceComponent19::ExtendedDescription()
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
    ServiceComponent19::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent19::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
