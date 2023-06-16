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

#ifndef CPPMICROSERVICES_SERVICEREGISTRATIONLOCKS_H
#define CPPMICROSERVICES_SERVICEREGISTRATIONLOCKS_H

#include "ServiceRegistrationBasePrivate.h"
#include "ServiceRegistrationCoreInfo.h"

namespace cppmicroservices
{

    /**
     * \ingroup MicroServices
     * This class is designed to store locks to the ServiceRegistrationBasePrivate object and the
     * ServiceRegistrationCoreInfo objects. This will lock and unlock them in order to avoid deadlocks. It also verifies
     * the aliveness of the ServiceRegistrationBasePrivate object as its lifetime is no longer linked to
     * ServiceReferenceBasePrivate and can therefore die while ServiceRegistrationCoreInfo is still needed.
     *
     * This is an RAII object, construct it in scope and when it goes out of scope the locks will be released.
     */
    class ServiceRegistrationLocks final
    {
      public:
        ServiceRegistrationLocks(const std::shared_ptr<ServiceRegistrationBasePrivate>& reg,
                                 const std::shared_ptr<ServiceRegistrationCoreInfo>& coreInfo);

        // Delete all copy and move to enforce that it is only ever constructed into one object -- avoids deadlocks
        ServiceRegistrationLocks(ServiceRegistrationLocks const& lockObj) = delete;
        ServiceRegistrationLocks(ServiceRegistrationLocks&& lockObj) = default;
        ServiceRegistrationLocks& operator=(ServiceRegistrationLocks const& lockObj) = delete;
        ServiceRegistrationLocks& operator=(ServiceRegistrationLocks&& lockObj) = delete;

      private:
        cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock regL;
        cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock coreInfoL;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREGISTRATIONLOCKS_H