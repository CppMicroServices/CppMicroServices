#ifndef BPUninstalledState_h
#define BPUninstalledState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPUninstalledState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual void Start(BundlePrivate&, uint32_t) override;
        virtual void Stop(BundlePrivate&, uint32_t) override;
        virtual void Uninstall(BundlePrivate&) override;

        uint32_t GetState() override {
            return 0x00000001;
        };

    };
} 

#endif
