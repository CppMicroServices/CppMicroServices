

#ifndef ConcurrencyTestUtil_hpp
#define ConcurrencyTestUtil_hpp

#include <algorithm>
#include <functional>
#include <future>
#include <vector>

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
    std::vector<std::future<R>> enable_async_futs(numCalls);
    for (size_t i = 0; i < numCalls; ++i)
    {
        enable_async_futs[i] = std::async(std::launch::async,
                                          [&readies, i, &ready, &func]()
                                          {
                                              readies[i].set_value();
                                              ready.wait();
                                              return func();
                                          });
    }

    for (size_t i = 0; i < numCalls; ++i)
    {
        readies[i].get_future().wait();
    }

    go.set_value();

    for (size_t i = 0; i < numCalls; ++i)
    {
        returnVals.push_back(enable_async_futs[i].get());
    }
    return returnVals;
}

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
    std::size_t generation;
};

#endif /* ConcurrencyTestUtil_hpp */
