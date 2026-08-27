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

        void Start(BundlePrivate&, uint32_t) override;
        void Stop(BundlePrivate&, uint32_t) override;
        void Uninstall(BundlePrivate&) override;

        uint32_t GetState() override;

    };
} 

#endif
