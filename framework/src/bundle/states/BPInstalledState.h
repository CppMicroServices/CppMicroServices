#ifndef BPInstalledState_h
#define BPInstalledState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPInstalledState final : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        ~BPInstalledState() override = default;
        BPInstalledState(BPInstalledState const&) = delete;
        BPInstalledState& operator=(BPInstalledState const&) = delete;
        BPInstalledState(BPInstalledState&&) = delete;
        BPInstalledState& operator=(BPInstalledState&&) = delete;

        void Start(BundlePrivate&, uint32_t) override;
        void Stop(BundlePrivate&, uint32_t) override;
        void Uninstall(BundlePrivate&) override;
    };
} 

#endif
