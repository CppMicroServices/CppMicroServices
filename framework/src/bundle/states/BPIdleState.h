#ifndef BPIdleState_h
#define BPIdleState_h

#include "BundlePrivateState.h"

namespace cppmicroservices
{
    class BPActiveState;
    class BPIdleState : public BundlePrivateState
    { 
        public:
        using BundlePrivateState::BundlePrivateState;
        void StartFromIdle(BundlePrivate&, std::shared_ptr<BPActiveState> const&);

    };
} 

#endif
