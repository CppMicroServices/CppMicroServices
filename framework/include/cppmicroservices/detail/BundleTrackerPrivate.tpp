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

#include "cppmicroservices/detail/TrackedBundle.h"

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"

#include <stdexcept>
#include <utility>

namespace cppmicroservices {

namespace detail {

template<class TTT>
BundleTrackerPrivate<TTT>::BundleTrackerPrivate(
  BundleTracker<T>* bt,
  const BundleContext& context,
  StateType stateMask,
  BundleTrackerCustomizer<T>* customizer)
  : context(std::move(context))
  , customizer(customizer)
  , stateMask(stateMask)
  , listenerToken()
  , trackedBundle()
  , q_ptr(bt)
{
  this->customizer = customizer ? customizer : q_func();
}

template<class TTT>
BundleTrackerPrivate<TTT>::~BundleTrackerPrivate() = default;

template<class TTT>
std::shared_ptr<detail::TrackedBundle<TTT>> BundleTrackerPrivate<TTT>::Tracked()
  const
{
  return trackedBundle.Load();
}

template<class TTT>
std::vector<Bundle> BundleTrackerPrivate<TTT>::GetInitialBundles(
  typename BundleTrackerPrivate<TTT>::StateType stateMask)
{
  std::vector<Bundle> result;
  std::vector<Bundle> contextBundles = context.GetBundles();
  for (Bundle bundle : contextBundles) {
    result.push_back(bundle);
  }
  return result;
}

template<class TTT>
void BundleTrackerPrivate<TTT>::GetBundles_unlocked(std::vector<Bundle>& refs,
                                                    TrackedBundle<TTT>* t) const
{
  if (t->Size_unlocked() == 0) {
    return;
  }
  t->GetTracked_unlocked(refs);
}

template<class TTT>
void BundleTrackerPrivate<TTT>::Modified()
{
  // No caching to clear
  DIAG_LOG(*context.GetLogSink()) << "BundleTracker::Modified(): " << stateMask;
}

} // namespace detail

} // namespace cppmicroservices
