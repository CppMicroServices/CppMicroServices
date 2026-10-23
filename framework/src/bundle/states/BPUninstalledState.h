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


        void Start(BundlePrivate&, uint32_t) override;
        void Stop(BundlePrivate&, uint32_t) override;
        void Uninstall(BundlePrivate&) override;

    };
} 

#endif
