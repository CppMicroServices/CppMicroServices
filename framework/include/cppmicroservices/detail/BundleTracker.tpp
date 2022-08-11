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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"

#include "cppmicroservices/detail/BundleTrackerPrivate.h"
#include "cppmicroservices/detail/TrackedBundle.h"

#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

namespace cppmicroservices {

// Destructor
template<class T>
BundleTracker<T>::~BundleTracker()
{
  try {
    Close();
  } catch (...) {
  }
}

template<class T>
BundleTracker<T>::BundleTracker(
  const BundleContext& context,
  const BundleStateMaskType stateMask,
  const std::shared_ptr<_BundleTrackerCustomizer> customizer)
  : d(std::make_unique<_BundleTrackerPrivate>(this,
                                              context,
                                              stateMask,
                                              customizer))
{}

template<class T>
void BundleTracker<T>::Open()
{
  std::shared_ptr<_TrackedBundle> t;
  {
    auto l = d->Lock();
    US_UNUSED(l);
    if (d->trackedBundle.Load() &&
        !d->Tracked()->closed) { /* If BundleTracker is open */
      return;
    }

    DIAG_LOG(*d->context.GetLogSink())
      << "BundleTracker<T>::Open: " << d->stateMask;

    t = std::make_shared<_TrackedBundle>(
      this, d->customizer ? d->customizer.get() : d->getTrackerAsCustomizer());
    try {
      // Attempt to drop old listener
      d->context.RemoveListener(std::move(d->listenerToken));
      // Make new listener
      d->listenerToken = d->context.AddBundleListener(
        std::bind(&_TrackedBundle::BundleChanged, t, std::placeholders::_1));

      std::vector<Bundle> bundles = d->GetInitialBundles(d->stateMask);
      t->SetInitial(bundles);
    } catch (const std::invalid_argument& e) {
      // Remove listener and rethrow
      d->context.RemoveListener(std::move(d->listenerToken));
      throw std::runtime_error(
        std::string("unexpected std::invalid_argument exception: ") + e.what());
    }
    d->trackedBundle.Store(t);
  }
  t->TrackInitial();
}

template<class T>
void BundleTracker<T>::Close() noexcept
{
  std::shared_ptr<_TrackedBundle> outgoing = d->trackedBundle.Load();
  {
    auto l = d->Lock();
    US_UNUSED(l);

    try {
      d->context.RemoveListener(std::move(d->listenerToken));
    } catch (const std::runtime_error&) {
      /* Rescue if context is stopped or invalid */
    }

    if (outgoing == nullptr) {
      return;
    }

    // Check for already closed
    if (d->Tracked()->closed) {
      return;
    }

    DIAG_LOG(*d->context.GetLogSink())
      << "BundleTracker<T>::close:" << d->stateMask;

    outgoing->Close();
    d->Modified();         /* log message */
    outgoing->NotifyAll(); /* wake up any waiters */
  }

  try {
    outgoing->WaitOnCustomizersToFinish();
  } catch (const std::exception&) {
    // Rescue CounterLatch issues
  }

  auto bundles = GetBundles();
  for (auto& bundle : bundles) {
    outgoing->Untrack(bundle, BundleEvent());
  }
}

template<class T>
std::vector<Bundle> BundleTracker<T>::GetBundles() const noexcept
{
  std::vector<Bundle> bundles;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return bundles;
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    d->GetBundles_unlocked(bundles, t.get());
  }
  return bundles;
}

template<class T>
std::optional<typename BundleTracker<T>::TrackedParamType>
BundleTracker<T>::GetObject(const Bundle& bundle) const noexcept
{
  auto t = d->Tracked();
  if (!t ||
      !bundle) { /* If BundleTracker is not open or if bundle is invalid */
    return std::nullopt;
  }
  return (t->Lock(), t->GetCustomizedObject_unlocked(bundle));
}

template<class T>
typename BundleTracker<T>::TrackingMap BundleTracker<T>::GetTracked()
  const noexcept
{
  BundleTracker<T>::TrackingMap trackingMap;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return trackingMap;
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    t->CopyEntries_unlocked(trackingMap);
  }
  return trackingMap;
}

template<class T>
int BundleTracker<T>::GetTrackingCount() const noexcept
{
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return -1;
  }
  return (t->Lock(), t->GetTrackingCount());
}

template<class T>
bool BundleTracker<T>::IsEmpty() const noexcept
{
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return true;
  }
  return (t->Lock(), t->IsEmpty_unlocked());
}

template<class T>
void BundleTracker<T>::Remove(const Bundle& bundle) noexcept
{
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return;
  }
  t->Untrack(bundle, BundleEvent());
}

template<class T>
size_t BundleTracker<T>::Size() const noexcept
{
  size_t size;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return 0;
  }
  return (t->Lock(), t->Size_unlocked());
}

template<class T>
std::optional<typename BundleTracker<T>::TrackedParamType>
BundleTracker<T>::AddingBundle(const Bundle& bundle, const BundleEvent&)
{
  return TypeTraits::ConvertToTrackedType(bundle);
}

template<class T>
void BundleTracker<T>::ModifiedBundle(const Bundle&,
                                      const BundleEvent&,
                                      const T&)
{
  /* do nothing */
}

template<class T>
void BundleTracker<T>::RemovedBundle(const Bundle&,
                                     const BundleEvent&,
                                     const T&)
{
  /* do nothing */
}
}
