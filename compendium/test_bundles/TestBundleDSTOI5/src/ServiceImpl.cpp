#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponent5::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponent5::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponent5::ExtendedDescription()
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

    bool ServiceComponent5::isBound(){
        return foo != nullptr;
    }

    void
    ServiceComponent5::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::cout << "BIND3 " << theFoo.get() << std::endl;
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponent5::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
