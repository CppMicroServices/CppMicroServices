#ifndef BPStoppingState_h
#define BPStoppingState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPStoppingState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        ~BPStoppingState() override = default;
        BPStoppingState(BPStoppingState const&) = delete;
        BPStoppingState& operator=(BPStoppingState const&) = delete;
        BPStoppingState(BPStoppingState&&) = delete;
        BPStoppingState& operator=(BPStoppingState&&) = delete;

        void Start(BundlePrivate& mgr, uint32_t options) override;

        void Stop(BundlePrivate& mgr, uint32_t options) override;

        void Uninstall(BundlePrivate& mgr) override;

        uint32_t GetState() override; 
    };
} 

#endif
