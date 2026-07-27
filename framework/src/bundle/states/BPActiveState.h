#ifndef BPActiveState_h
#define BPActiveState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPActiveState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        ~BPActiveState() override = default;
        BPActiveState(BPActiveState const&) = delete;
        BPActiveState& operator=(BPActiveState const&) = delete;
        BPActiveState(BPActiveState&&) = delete;
        BPActiveState& operator=(BPActiveState&&) = delete;

        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;

        uint32_t GetState() override;

    };
} 

#endif
