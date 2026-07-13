#ifndef BPRunningState_h
#define BPRunningState_h

#include "BundlePrivateState.h"


namespace cppmicroservices
{
    class BPRunningState : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        virtual ~BPRunningState() = default;
        void Stop(BundlePrivate& mgr, uint32_t options) override;
        void Uninstall(BundlePrivate& mgr) override;
    };
} 

#endif
