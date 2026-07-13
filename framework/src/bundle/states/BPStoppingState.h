#ifndef BPStoppingState_h
#define BPStoppingState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPStoppingState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPStoppingState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override;

        void Stop(BundlePrivate& mgr, uint32_t options) override;

        void Uninstall(BundlePrivate& mgr) override;

        uint32_t GetState() override {
            return 0x00000010;
        };

    };
} 

#endif
