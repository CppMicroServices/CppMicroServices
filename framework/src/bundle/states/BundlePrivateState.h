#ifndef BundlePrivateState_h
#define BundlePrivateState_h

#include <cstdint>
#include <future>
#include <memory>
#include <string>

namespace cppmicroservices
{
    class BundlePrivate;
    class BundlePrivateState;
    struct FrameworkShutdownBlocker;

    class TransitionLogger
    {
    public:
        TransitionLogger(BundlePrivate& mgr, std::string transitionName, uint32_t expectedState);
        ~TransitionLogger();

        TransitionLogger(TransitionLogger const&) = delete;
        TransitionLogger& operator=(TransitionLogger const&) = delete;
        TransitionLogger(TransitionLogger&&) = delete;
        TransitionLogger& operator=(TransitionLogger&&) = delete;

        void TransitionSucceeded();
        void SetActualState(std::shared_ptr<BundlePrivateState> const& state);

    private:
        BundlePrivate& mgr;
        std::string transitionName;
        uint32_t expectedState;
        uint32_t actualState;
        bool successfulTransition;
        int uncaughtExceptionCount;
    };

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
        std::unique_ptr<FrameworkShutdownBlocker> CheckAndBlockFramework(BundlePrivate& mgr);

        void WaitForTransitionTask();

        private:
        std::shared_future<void> ready;
        std::thread::id ownerThread;

    };

} 

#endif 
