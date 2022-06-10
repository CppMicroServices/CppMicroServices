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

#ifndef CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H
#define CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/detail/Threads.h"

namespace cppmicroservices {

namespace detail {

/**
 * \ingroup MicroServices
 */
template<class TTT>
class BundleTrackerPrivate : MultiThreaded<>
{

public:
  using T = typename TTT::TrackedType;
  using TrackedParamType = typename TTT::TrackedParamType;

  BundleTrackerPrivate(std::weak_ptr<BundleTracker<T>>,
                       const BundleContext& context,
                       std::weak_ptr<BundleTrackerCustomizer<T>> customizer);
  ~BundleTrackerPrivate();
};

}
}

#endif // CPPMICROSERVICES_BUNDLETRACKERPRIVATE_H