#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentDynamicGreedyMandatoryUnary::Activate(std::shared_ptr<ComponentContext> const& /*ctxt*/)
    {
    }

    void
    ServiceComponentDynamicGreedyMandatoryUnary::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponentDynamicGreedyMandatoryUnary::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (!foo)
        {
            throw std::runtime_error("Dependency not available");
        }
        std::string result("ServiceComponentDynamicGreedyMandatoryUnary ");
        result.append("depends on ");
        result.append(foo->Description());
        return result;
    }

    bool ServiceComponentDynamicGreedyMandatoryUnary::isBound(){
        return foo != nullptr;
    }


    void
    ServiceComponentDynamicGreedyMandatoryUnary::Bindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::cout << "BIND1 " << theFoo.get() << std::endl;
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo != theFoo)
        {
            foo = theFoo;
        }
    }

    void
    ServiceComponentDynamicGreedyMandatoryUnary::Unbindfoo(std::shared_ptr<test::Interface1> const& theFoo)
    {
        std::lock_guard<std::mutex> lock(fooMutex);
        if (foo == theFoo)
        {
            foo = nullptr;
        }
    }

} // namespace sample
