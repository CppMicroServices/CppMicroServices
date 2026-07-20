#ifndef BPStartingState_h
#define BPStartingState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPStartingState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPStartingState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;

        uint32_t GetState() override {
            return 0x00000008;
        };
    };
} 

#endif
