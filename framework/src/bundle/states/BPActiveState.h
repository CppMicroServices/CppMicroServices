#ifndef BPActiveState_h
#define BPActiveState_h

#include "BPRunningState.h"

namespace cppmicroservices
{
    class BPActiveState final : public BPRunningState
    { 
        public:
        using BPRunningState::BPRunningState;
        virtual ~BPActiveState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override{
            CheckFrameworkHasStopped(mgr);
            US_UNUSED(options);
            return;
        };

        uint32_t GetState() override {
            return 0x00000020;
        };

    };
} 

#endif
