#ifndef BPIdleState_h
#define BPIdleState_h

#include "BundlePrivateState.h"


namespace cppmicroservices
{
    class BPIdleState : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPIdleState() = default;
        void Stop(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;
    };
} 

#endif
