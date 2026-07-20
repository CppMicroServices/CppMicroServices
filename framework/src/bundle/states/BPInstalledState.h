#ifndef BPInstalledState_h
#define BPInstalledState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPInstalledState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPInstalledState() = default;

        void Start(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;
        void Stop(BundlePrivate& mgr, uint32_t options) override{
            SetAutostart(mgr, options);
            return;
        };

        uint32_t GetState() override {
            return 0x00000002;
        };

    };
} 

#endif
