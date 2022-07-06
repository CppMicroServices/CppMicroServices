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

#include <optional>
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
BundleTracker<T>::BundleTracker(const BundleContext& context,
                                StateType stateMask,
                                _BundleTrackerCustomizer* customizer)
  : d(new _BundleTrackerPrivate(this, context, stateMask, customizer))
{
}

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

    t.reset(new _TrackedBundle(this, d->customizer));
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
void BundleTracker<T>::Close()
{
  try {
    d->context.RemoveListener(std::move(d->listenerToken));
  } catch (const std::runtime_error&) {
    /* Rescue if context is stopped or invalid */
  }

  std::shared_ptr<_TrackedBundle> outgoing;
  outgoing = d->trackedBundle.Load();
  {
    auto l = d->Lock();
    US_UNUSED(l);

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
    outgoing->NotifyAll();
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
std::vector<Bundle> BundleTracker<T>::GetBundles()
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
typename BundleTracker<T>::TrackedParamType BundleTracker<T>::GetObject(
  const Bundle& bundle)
{
  typename BundleTracker<T>::TrackedParamType object;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return BundleTracker<T>::TrackedParamType();
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    object = t->GetCustomizedObject_unlocked(bundle);
  }
  return object;
}

template<class T>
typename BundleTracker<T>::TrackingMap BundleTracker<T>::GetTracked()
{
  BundleTracker<T>::TrackingMap map;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return map;
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    t->CopyEntries_unlocked(map);
  }
  return map;
}

template<class T>
int BundleTracker<T>::GetTrackingCount()
{
  int count;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return -1;
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    count = t->GetTrackingCount();
  }
  return count;
}

template<class T>
bool BundleTracker<T>::IsEmpty()
{
  bool isEmpty;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return true;
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    isEmpty = t->IsEmpty_unlocked();
  }
  return isEmpty;
}

template<class T>
void BundleTracker<T>::Remove(const Bundle& bundle)
{
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return;
  }
  t->Untrack(bundle, BundleEvent());
}

template<class T>
size_t BundleTracker<T>::Size()
{
  size_t size;
  auto t = d->Tracked();
  if (!t) { /* If BundleTracker is not open */
    return 0;
  }
  {
    auto l = t->Lock();
    US_UNUSED(l);
    size = t->Size_unlocked();
  }
  return size;
}

template<class T>
std::optional<typename BundleTracker<T>::TrackedParamType>
BundleTracker<T>::AddingBundle(const Bundle& bundle, const BundleEvent& event)
{
  // TODO: Make this SFINAE
  return TypeTraits::ConvertToTrackedType(bundle);
}

template<class T>
void BundleTracker<T>::ModifiedBundle(const Bundle& bundle,
                                      const BundleEvent& event,
                                      BundleTracker<T>::TrackedParamType object)
{
  /* do nothing */
}

template<class T>
void BundleTracker<T>::RemovedBundle(const Bundle& bundle,
                                     const BundleEvent& event,
                                     BundleTracker<T>::TrackedParamType object)
{
  /* do nothing */
}
}
