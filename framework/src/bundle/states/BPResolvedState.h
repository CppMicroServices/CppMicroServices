#ifndef BPResolvedState_h
#define BPResolvedState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPResolvedState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPResolvedState() = default;
        
        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override{
            SetAutostart(mgr, options);
            return;
        };

        virtual uint32_t GetState() override {
            return 0x00000004;
        };

    };
} 

#endif
