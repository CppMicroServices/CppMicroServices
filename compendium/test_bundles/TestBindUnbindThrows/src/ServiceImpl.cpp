#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    std::string
    ServiceComponentDGMU::ExtendedDescription()
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
    ServiceComponentDGMU::Bindfoo(std::shared_ptr<test::Interface1> const&)
    {
        throw std::runtime_error("throw from bind method");
    }

    void
    ServiceComponentDGMU::Unbindfoo(std::shared_ptr<test::Interface1> const&)
    {
        throw std::runtime_error("throw from unbind method");
    }

    std::string
    ServiceComponentDGOU::ExtendedDescription()
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
    ServiceComponentDGOU::Bindbar(std::shared_ptr<test::Interface1> const&)
    {
        throw std::runtime_error("throw from bind method");
    }

    void
    ServiceComponentDGOU::Unbindbar(std::shared_ptr<test::Interface1> const&)
    {

        throw std::runtime_error("throw from unbind method");
    }

    std::string
    ServiceComponentFactory::ExtendedDescription()
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
    ServiceComponentFactory::Bindfactory(std::shared_ptr<test::Interface1> const&)
    {
        throw std::runtime_error("throw from bind method");
    }

    void
    ServiceComponentFactory::Unbindfactory(std::shared_ptr<test::Interface1> const&)
    {

        throw std::runtime_error("throw from unbind method");
    }

} // namespace sample
