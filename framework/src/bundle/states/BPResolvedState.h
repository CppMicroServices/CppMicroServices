#ifndef BPResolvedState_h
#define BPResolvedState_h

#include "BPIdleState.h"

namespace cppmicroservices
{
    class BPResolvedState final : public BPIdleState
    { 
        public:
        using BPIdleState::BPIdleState;
        ~BPResolvedState() override = default;
        BPResolvedState(BPResolvedState const&) = delete;
        BPResolvedState& operator=(BPResolvedState const&) = delete;
        BPResolvedState(BPResolvedState&&) = delete;
        BPResolvedState& operator=(BPResolvedState&&) = delete;
        
        void Start(BundlePrivate&, uint32_t) override;
        void Uninstall(BundlePrivate&) override;
        void Stop(BundlePrivate&, uint32_t) override;

        uint32_t GetState() override;

    };
} 

#endif
