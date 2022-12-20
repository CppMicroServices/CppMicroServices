#include "ServiceImpl.hpp"

namespace sample
{

    void
    ServiceComponent9::Activate(std::shared_ptr<ComponentContext> const&)
    {
        activated = true;
        throw std::runtime_error("Exception thrown from user code");
    };
    void
    ServiceComponent9::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
        deactivated = true;
        throw std::runtime_error("Exception thrown from user code");
    };

} // namespace sample
