#ifndef BPResolvedState_h
#define BPResolvedState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPResolvedState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        ~BPResolvedState() override = default;
        BPResolvedState(BPResolvedState const&) = delete;
        BPResolvedState& operator=(BPResolvedState const&) = delete;
        BPResolvedState(BPResolvedState&&) = delete;
        BPResolvedState& operator=(BPResolvedState&&) = delete;
        
        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override;

        uint32_t GetState() override;

    };
} 

#endif
