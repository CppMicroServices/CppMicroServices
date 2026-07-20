#ifndef BPActiveState_h
#define BPActiveState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPActiveState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPActiveState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;

        uint32_t GetState() override {
            return 0x00000020;
        };

    };
} 

#endif
