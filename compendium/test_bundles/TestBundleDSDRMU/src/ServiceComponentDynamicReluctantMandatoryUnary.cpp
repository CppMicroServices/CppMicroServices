#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentDynamicReluctantMandatoryUnary::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponentDynamicReluctantMandatoryUnary::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponentDynamicReluctantMandatoryUnary::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result("ServiceComponentDynamicReluctantMandatoryUnary ");
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    bool ServiceComponentDynamicReluctantMandatoryUnary::isBound(){
        return foo != nullptr;
    }

    void
    ServiceComponentDynamicReluctantMandatoryUnary::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::cout << "BIND2" << std::endl;
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentDynamicReluctantMandatoryUnary::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
