#ifndef BPStartingState_h
#define BPStartingState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPStartingState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        ~BPStartingState() override = default;
        BPStartingState(BPStartingState const&) = delete;
        BPStartingState& operator=(BPStartingState const&) = delete;
        BPStartingState(BPStartingState&&) = delete;
        BPStartingState& operator=(BPStartingState&&) = delete;

        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;
        void StartFailed(BundlePrivate& mgr);

        uint32_t GetState() override;
    };
} 

#endif
