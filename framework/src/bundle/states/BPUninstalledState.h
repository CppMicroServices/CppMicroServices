#ifndef BPUninstalledState_h
#define BPUninstalledState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPUninstalledState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        ~BPUninstalledState() override = default;
        BPUninstalledState(BPUninstalledState const&) = delete;
        BPUninstalledState& operator=(BPUninstalledState const&) = delete;
        BPUninstalledState(BPUninstalledState&&) = delete;
        BPUninstalledState& operator=(BPUninstalledState&&) = delete;


        virtual void Start(BundlePrivate&, uint32_t) override;
        virtual void Stop(BundlePrivate&, uint32_t) override;
        virtual void Uninstall(BundlePrivate&) override;

        uint32_t GetState() override;

    };
} 

#endif
