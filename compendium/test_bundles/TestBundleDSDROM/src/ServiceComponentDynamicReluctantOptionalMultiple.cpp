#include "ServiceImpl.hpp"

#include <algorithm>
#include <sstream>

namespace sample
{

    std::atomic<int> ServiceComponentDynamicReluctantOptionalMultiple::nullUnbindCount_ {0};

    ServiceComponentDynamicReluctantOptionalMultiple::ServiceComponentDynamicReluctantOptionalMultiple(
        std::shared_ptr<test::Interface1> const& dep)
        : staticDep_(dep)
    {
    }

    void
    ServiceComponentDynamicReluctantOptionalMultiple::Activate(std::shared_ptr<ComponentContext> const&)
    {
    }

    void
    ServiceComponentDynamicReluctantOptionalMultiple::Deactivate(std::shared_ptr<ComponentContext> const&)
    {
    }

    std::string
    ServiceComponentDynamicReluctantOptionalMultiple::ExtendedDescription()
    {
        std::lock_guard<std::mutex> lock(optionalDepsMutex_);
        std::ostringstream oss;
        oss << "bindCount=" << bindCount_.load()
            << " nullBindCount=" << nullBindCount_.load()
            << " unbindCount=" << unbindCount_.load()
            << " nullUnbindCount=" << nullUnbindCount_.load()
            << " optionalDeps=" << optionalDeps_.size();
        return oss.str();
    }

    void
    ServiceComponentDynamicReluctantOptionalMultiple::BindoptionalDeps(
        std::shared_ptr<test::Interface3> const& dep)
    {
        if (!dep)
        {
            nullBindCount_++;
            return;
        }
        bindCount_++;
        std::lock_guard<std::mutex> lock(optionalDepsMutex_);
        optionalDeps_.push_back(dep);
    }

    void
    ServiceComponentDynamicReluctantOptionalMultiple::UnbindoptionalDeps(
        std::shared_ptr<test::Interface3> const& dep)
    {
        if (!dep)
        {
            nullUnbindCount_++;
            return;
        }
        unbindCount_++;
        std::lock_guard<std::mutex> lock(optionalDepsMutex_);
        optionalDeps_.erase(
            std::remove(optionalDeps_.begin(), optionalDeps_.end(), dep),
            optionalDeps_.end());
    }

} // namespace sample
