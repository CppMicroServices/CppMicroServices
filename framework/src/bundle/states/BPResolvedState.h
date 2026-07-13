#ifndef BPResolvedState_h
#define BPResolvedState_h

#include "BPIdleState.h"

namespace cppmicroservices
{
    class BPResolvedState final : public BPIdleState
    { 
        public:
        using BPIdleState::BPIdleState;
        virtual ~BPResolvedState() = default;
        
        void Start(BundlePrivate& mgr, uint32_t options) override;

        virtual uint32_t GetState() override {
            return 0x00000004;
        };

    };
} 

#endif
