#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponent7::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponent7::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponent7::ExtendedDescription()
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

    bool ServiceComponent7::isBound(){
        return foo != nullptr;
    }

    void
    ServiceComponent7::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::cout << "BIND4 " << theFoo.get() << std::endl;
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent7::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
