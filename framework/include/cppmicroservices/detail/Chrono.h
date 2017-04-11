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

#ifndef CPPMICROSERVICES_CHRONO_H
#define CPPMICROSERVICES_CHRONO_H

#include <chrono>

namespace cppmicroservices {

namespace detail {

#if !defined(__clang__) && __GNUC__ == 4 && __GNUC_MINOR__ < 7
typedef std::chrono::monotonic_clock Clock;
#else
typedef std::chrono::steady_clock Clock;
#endif

}

}

#endif // CPPMICROSERVICES_CHRONO_H
