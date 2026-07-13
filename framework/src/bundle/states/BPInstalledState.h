#ifndef BPInstalledState_h
#define BPInstalledState_h

#include "BPIdleState.h"

namespace cppmicroservices
{
    class BPInstalledState final : public BPIdleState
    { 
        public:
        using BPIdleState::BPIdleState;
        virtual ~BPInstalledState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override;

        uint32_t GetState() override {
            return 0x00000002;
        };

    };
} 

#endif
