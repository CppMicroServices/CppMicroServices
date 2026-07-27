#ifndef BundlePrivateState_h
#define BundlePrivateState_h

#include <cstdint>
#include <future>
#include <memory>

namespace cppmicroservices
{
    class BundlePrivate;

    class BundlePrivateState : public std::enable_shared_from_this<BundlePrivateState>
    { 
    
        public:
        BundlePrivateState();
        explicit BundlePrivateState(std::shared_future<void> blockUntil);
        virtual ~BundlePrivateState() = default;
        BundlePrivateState(BundlePrivateState const&) = delete;
        BundlePrivateState& operator=(BundlePrivateState const&) = delete;
        BundlePrivateState(BundlePrivateState&&) = delete;
        BundlePrivateState& operator=(BundlePrivateState&&) = delete;


        virtual void Start(BundlePrivate& mgr, uint32_t options)=0;
        virtual void Stop(BundlePrivate& mgr, uint32_t options)=0;
        virtual void Uninstall(BundlePrivate& mgr)=0;

        virtual uint32_t GetState()=0;

        void SetAutostart(BundlePrivate& mgr, uint32_t options);
        void CheckFrameworkHasStopped(BundlePrivate& mgr);

        void WaitForTransitionTask();

        private:
        std::shared_future<void> ready;

    };

} 

#endif 
