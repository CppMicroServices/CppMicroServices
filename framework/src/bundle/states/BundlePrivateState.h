#ifndef BundlePrivateState_h
#define BundlePrivateState_h

#include <cstdint>
#include <future>
#include <memory>
#include <string>
#include <thread>

namespace cppmicroservices
{
    class BundlePrivate;
    class BundlePrivateState;
    struct FrameworkShutdownBlocker;

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


        virtual void Start(BundlePrivate&, uint32_t)=0;
        virtual void Stop(BundlePrivate&, uint32_t)=0;
        virtual void Uninstall(BundlePrivate& mgr)=0;

        virtual uint32_t GetState()=0;

        void SetAutostart(BundlePrivate&, uint32_t, uint32_t);
        std::unique_ptr<FrameworkShutdownBlocker> CheckAndBlockFramework(BundlePrivate&);

        void WaitForTransitionTask();

        private:
        std::shared_future<void> ready;
        std::thread::id ownerThread;

    };

    class TransitionCompletionGuard
    {
      public:
        explicit TransitionCompletionGuard(std::promise<void>& transitionAction);
        ~TransitionCompletionGuard();

        TransitionCompletionGuard(TransitionCompletionGuard const&) = delete;
        TransitionCompletionGuard& operator=(TransitionCompletionGuard const&) = delete;
        TransitionCompletionGuard(TransitionCompletionGuard&&) = delete;
        TransitionCompletionGuard& operator=(TransitionCompletionGuard&&) = delete;

        void Complete();

      private:
        std::promise<void>* transitionAction;
    };

    class TransitionLogger
    {
    public:
        TransitionLogger(BundlePrivate& mgr, std::string transitionName, uint32_t expectedState);
        ~TransitionLogger();

        TransitionLogger(TransitionLogger const&) = delete;
        TransitionLogger& operator=(TransitionLogger const&) = delete;
        TransitionLogger(TransitionLogger&&) = delete;
        TransitionLogger& operator=(TransitionLogger&&) = delete;

        void MarkTransitionAccepted();
        void SetActualState(std::shared_ptr<BundlePrivateState> const& state);

    private:
        BundlePrivate& mgr;
        std::string transitionName;
        uint32_t expectedState;
        uint32_t actualState;
        bool successfulTransition;
        int uncaughtExceptionCount;
    };

} 

#endif 
