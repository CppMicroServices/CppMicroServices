#ifndef BPInstalledState_h
#define BPInstalledState_h

#include "BPIdleState.h"

namespace cppmicroservices
{
    class BPInstalledState final : public BPIdleState
    { 
        public:
        using BPIdleState::BPIdleState;
        ~BPInstalledState() override = default;
        BPInstalledState(BPInstalledState const&) = delete;
        BPInstalledState& operator=(BPInstalledState const&) = delete;
        BPInstalledState(BPInstalledState&&) = delete;
        BPInstalledState& operator=(BPInstalledState&&) = delete;

        void Start(BundlePrivate&, uint32_t) override;
        void Stop(BundlePrivate&, uint32_t) override;
        void Uninstall(BundlePrivate&) override;

        uint32_t GetState() override;

    };
} 

#endif
