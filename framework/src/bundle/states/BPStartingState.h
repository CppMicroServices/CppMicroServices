#ifndef BPStartingState_h
#define BPStartingState_h

#include "BPRunningState.h"

namespace cppmicroservices
{
    class BPStartingState final : public BPRunningState
    { 
        public:
        using BPRunningState::BPRunningState;
        virtual ~BPStartingState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override;


        uint32_t GetState() override {
            return 0x00000008;
        };
    };
} 

#endif
