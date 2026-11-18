#include "ServiceImpl.hpp"

#include <algorithm>

namespace sample
{
    void
    MultiInterfaceConsumer::BindmiA(std::shared_ptr<test::MultiInterfaceA> const& a)
    {
        if (!a)
        {
            return;
        }
        std::lock_guard<std::mutex> lock(boundAMutex_);
        boundA_.push_back(a);
    }

    void
    MultiInterfaceConsumer::UnbindmiA(std::shared_ptr<test::MultiInterfaceA> const& a)
    {
        if (!a)
        {
            return;
        }
        std::lock_guard<std::mutex> lock(boundAMutex_);
        boundA_.erase(std::remove(boundA_.begin(), boundA_.end(), a), boundA_.end());
    }

    std::size_t
    MultiInterfaceConsumer::BoundCount()
    {
        std::lock_guard<std::mutex> lock(boundAMutex_);
        return boundA_.size();
    }

    std::string
    MultiInterfaceConsumer::BoundIdentity()
    {
        std::lock_guard<std::mutex> lock(boundAMutex_);
        if (boundA_.empty())
        {
            return "<none>";
        }
        // Dispatch through the bound MultiInterfaceA pointer. If the reference
        // was bound to the correct sub-object this returns "A"; if it received
        // the MultiInterfaceB sub-object (mis-adjusted pointer) the call
        // dispatches through MultiInterfaceB's vtable and returns "B".
        return boundA_.front()->WhoA();
    }

} // namespace sample
