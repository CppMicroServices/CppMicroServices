/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "ThreadpoolSafeFuturePrivate.hpp"
#include <chrono>
#include <future>

namespace cppmicroservices::cmimpl
{
    ThreadpoolSafeFuturePrivate::ThreadpoolSafeFuturePrivate(std::shared_future<void> future,
                                                             std::shared_ptr<SingleInvokeTask> task)
        : future(std::move(future))
        , task(std::move(task))
    {
    }
    void
    ThreadpoolSafeFuturePrivate::get() const
    {
        return wait();
    }
    void
    ThreadpoolSafeFuturePrivate::wait() const
    {
        constexpr auto timeout = std::chrono::milliseconds(50);
        // if we hit the timeout
        if (future.wait_for(timeout) == std::future_status::timeout)
        {
            // we expect that the task is stalled
            // if the task is not stalled, this is a noop
            (*task)();
        }

        // we can always get the future... iit was either executed by the GTP thread or on this one
        future.get();
    }

    std::future_status
    ThreadpoolSafeFuturePrivate::wait_for(std::uint32_t const& timeout_duration_ms) const
    {
        return future.wait_for(std::chrono::milliseconds(timeout_duration_ms));
    }

} // namespace cppmicroservices::cmimpl
