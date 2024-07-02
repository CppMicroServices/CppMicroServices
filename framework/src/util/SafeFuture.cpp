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

#include "cppmicroservices/SafeFuture.h"

namespace cppmicroservices
{
    SafeFuture::SafeFuture(std::shared_future<void> future,
                           std::shared_ptr<std::atomic<bool>> asyncStarted,
                           std::shared_ptr<ActualTask> task)
        : future(future)
        , asyncStarted(asyncStarted)
        , task(task)
    {
    }
    void
    SafeFuture::get() const
    {
        return wait();
    }
    void
    SafeFuture::wait() const
    {
        // ensure that asyncTaskMap and asyncTaskStateMap are cleared even if task throws

        constexpr auto timeout = std::chrono::milliseconds(50);
        // if we hit the timeout
        if (future.wait_for(timeout) == std::future_status::timeout)
        {
            // we expect that the asyncStarted is false -- i.e. stalled
            auto expected = false;
            auto desired = true;
            // if it is *asyncStarted==false
            if (std::atomic_compare_exchange_strong(&(*asyncStarted), &expected, desired))
            {
                // we pass in false because we always want to execute the task here
                (*task)(false);
            }
        }

        // we can always get the future... if stalled, it'll be satisfied by WFF
        // execution of the task else it will be satisfied by already executed object
        future.get();
    }

} // namespace cppmicroservices
