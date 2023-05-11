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

#ifndef CPPMICROSERVICES_REGISTRATIONLOCKS_H
#define CPPMICROSERVICES_REGISTRATIONLOCKS_H

#include "ServiceRegistrationBasePrivate.h"
#include "ServiceRegistrationCoreInfo.h"

namespace cppmicroservices
{

    /**
     * \ingroup MicroServices
     */
    class RegistrationLocks
    {
        public:
            RegistrationLocks(std::shared_ptr<ServiceRegistrationBasePrivate> reg, std::shared_ptr<ServiceRegistrationCoreInfo> coreInfo);
            
            ~RegistrationLocks();
        private:
#ifdef US_ENABLE_THREADING_SUPPORT
            cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock coreInfoL;
            cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock regL;
#endif
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_REGISTRATIONLOCKS_H