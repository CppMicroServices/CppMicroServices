

#ifndef ConcurrencyTestUtil_hpp
#define ConcurrencyTestUtil_hpp

#include <algorithm>
#include <functional>
#include <future>
#include <vector>

namespace cppmicroservices
{
    namespace detail
    {
        namespace test
        {

        class Barrier
        {
        public:
            Barrier(std::size_t count) : threshold(count), count(count), generation(0) {}

            void
            Wait()
            {
                std::unique_lock<std::mutex> lock(mutex);
                auto gen = generation;
                if (--count == 0)
                {
                    generation++;
                    count = threshold;
                    cond.notify_all();
                }
                else
                {
                    cond.wait(lock, [this, gen] { return gen != generation; });
                }
            }

        private:
            std::mutex mutex;
            std::condition_variable cond;
            std::size_t threshold;
            std::size_t count;
            std::size_t generation{0};
        };
            /**
             * Util method to determine if a future object is ready
             */
            template <typename T>
            bool
            is_ready(T&& futObj)
            {
                return (futObj.wait_for(std::chrono::seconds::zero()) == std::future_status::ready);
            }

            /**
             * Utility method for concurrently running a function
             */
            template <class R, class... Args>
            std::vector<R>
            ConcurrentInvoke(std::function<R(Args...)> func)
            {
                // test concurrent calls to enable and disable from multiple threads
                std::vector<R> returnVals;
                std::promise<void> go;
                std::shared_future<void> ready(go.get_future());
                size_t numCalls = std::max(64u, std::thread::hardware_concurrency());
                std::vector<std::promise<void>> readies(numCalls);

                // TODO: use std::barrier when available
                Barrier beforeWork(numCalls);

                std::vector<std::future<R>> enable_async_futs(numCalls);
                for (size_t i = 0; i < numCalls; ++i)
                {
                    enable_async_futs[i] = std::async(std::launch::async,
                                                      [&beforeWork, &func]()
                                                      {
                                                          beforeWork.Wait();
                                                          return func();
                                                      });
                }

                for (size_t i = 0; i < numCalls; ++i)
                {
                    returnVals.push_back(enable_async_futs[i].get());
                }
                return returnVals;
            }

        } // namespace test
    }     // namespace detail
} // namespace cppmicroservices

#endif /* ConcurrencyTestUtil_hpp */
