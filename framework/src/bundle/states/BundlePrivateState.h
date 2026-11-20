#ifndef BundlePrivateState_h
#define BundlePrivateState_h

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace cppmicroservices
{
    class BundlePrivate;
    class BundlePrivateState;
    struct FrameworkShutdownBlocker;
    class BPActiveState;

    class BundlePrivateState : public std::enable_shared_from_this<BundlePrivateState>
    {
        public:
        // Pass to the constructor when the state is the target of an in-flight
        // transition. Such a state starts out NOT yet complete; the thread performing
        // the transition signals completion via SignalTransitionComplete() (normally
        // through a TransitionCompletionGuard). The default (false) creates an idle
        // state with no producing transition to wait for.
        static constexpr bool PENDING_TRANSITION = true;

        explicit BundlePrivateState(bool pendingTransition = false);
        virtual ~BundlePrivateState() = default;
        BundlePrivateState(BundlePrivateState const&) = delete;
        BundlePrivateState& operator=(BundlePrivateState const&) = delete;
        BundlePrivateState(BundlePrivateState&&) = delete;
        BundlePrivateState& operator=(BundlePrivateState&&) = delete;


        virtual void Start(BundlePrivate&, uint32_t)=0;
        virtual void Stop(BundlePrivate&, uint32_t)=0;
        virtual void Uninstall(BundlePrivate& mgr)=0;

        void SetAutostart(BundlePrivate&, uint32_t, uint32_t);
        std::unique_ptr<FrameworkShutdownBlocker> CheckAndBlockFramework(BundlePrivate&);
        void StartFromIdle(BundlePrivate&, std::shared_ptr<BPActiveState> const&);

        // Block until the transition that produced this state object has finished
        // its work. Lock-free fast path once completion has been signalled.
        void WaitForTransitionTask();

        // Mark the transition that produced this state object as complete, waking
        // any threads blocked in WaitForTransitionTask(). Idempotent.
        void SignalTransitionComplete();

        private:
        // One-shot completion latch for this state's producing transition. The
        // atomic flag is the common-case fast path; the mutex/condition_variable
        // are touched only when a thread actually has to block. Replaces the
        // per-transition heap-allocated std::promise/std::shared_future baton.
        std::mutex transitionMutex;
        std::condition_variable transitionCv;
        std::atomic<bool> transitionComplete;
        std::thread::id ownerThread;

    };

    class TransitionCompletionGuard
    {
      public:
        explicit TransitionCompletionGuard(BundlePrivateState& state);
        ~TransitionCompletionGuard();

        TransitionCompletionGuard(TransitionCompletionGuard const&) = delete;
        TransitionCompletionGuard& operator=(TransitionCompletionGuard const&) = delete;
        TransitionCompletionGuard(TransitionCompletionGuard&&) = delete;
        TransitionCompletionGuard& operator=(TransitionCompletionGuard&&) = delete;

        void Complete();

      private:
        BundlePrivateState* state;
    };

    class TransitionLogger
    {
    public:
        TransitionLogger(BundlePrivate& mgr, std::string transitionName);
        ~TransitionLogger();

        TransitionLogger(TransitionLogger const&) = delete;
        TransitionLogger& operator=(TransitionLogger const&) = delete;
        TransitionLogger(TransitionLogger&&) = delete;
        TransitionLogger& operator=(TransitionLogger&&) = delete;

        void MarkTransitionAccepted();

    private:
        BundlePrivate& mgr;
        std::string transitionName;
        bool successfulTransition;
        int uncaughtExceptionCount;
    };

}

#endif
