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

namespace cppmicroservices {

namespace detail {

template<class T>
TrackedBundle<T>::TrackedBundle(BundleTracker<T>* _bundleTracker,
                                BundleTrackerCustomizer<T>* _customizer)
  : BundleAbstractTracked<Bundle, T, BundleEvent>(_bundleTracker->d->context)
  , bundleTracker(_bundleTracker)
  , customizer(_customizer)
  , latch{}
{}

template<class T>
void TrackedBundle<T>::WaitOnCustomizersToFinish()
{
  latch.Wait();
}

template<class T>
void TrackedBundle<T>::BundleChanged(const BundleEvent& event)
{
  // Call track or untrack based on state mask

  (void)latch.CountUp();
  ScopeGuard sg([this]() {
    // By using try/catch here, we ensure that this lambda function doesn't
    // throw inside ScopeGuard
    try {
      latch.CountDown();
    } catch (...) {
    }
  });

  // Check trivial calls
  Bundle bundle = event.GetBundle();
  if (!bundle) {
    return;
  }
  Bundle::State state = bundle.GetState();
  if (!state) {
    return;
  }
  // Ignore events that do not correspond with
  // Bundle state changes
  BundleEvent::Type eventType = event.GetType();
  if (eventType == BundleEvent::Type::BUNDLE_UNRESOLVED) {
    return;
  }
  {
    auto l = this->Lock();
    US_UNUSED(l);

    // Check for delayed call
    if (this->closed) {
      return;
    }

    DIAG_LOG(*bundleTracker->d->context.GetLogSink())
      << "TrackedService::BundleChanged[" << state << "]: " << bundle;
  }

  // Track iff state in mask
  if (state & bundleTracker->d->stateMask) {
    /*
     * The below method will throw if a customizer throws,
     * and the exception will propagate to the listener.
     */
    this->Track(bundle, event);
  } else {
    /*
     * The below method will throw if a customizer throws,
     * and the exception will propagate to the listener.
     */
    this->Untrack(bundle, event);
  }
}

template<class T>
void TrackedBundle<T>::Modified()
{
  BundleAbstractTracked<Bundle, T, BundleEvent>::
    Modified(); /* increment the modification count */
  bundleTracker->d->Modified();
}

template<class T>
std::optional<T> TrackedBundle<T>::CustomizerAdding(Bundle bundle,
                                                    const BundleEvent& event)
{
  return customizer->AddingBundle(bundle, event);
}

template<class T>
void TrackedBundle<T>::CustomizerModified(Bundle bundle,
                                          const BundleEvent& event,
                                          const T& object)
{
  customizer->ModifiedBundle(bundle, event, object);
}

template<class T>
void TrackedBundle<T>::CustomizerRemoved(Bundle bundle,
                                         const BundleEvent& event,
                                         const T& object)
{
  customizer->RemovedBundle(bundle, event, object);
}

} // namespace detail

} // namespace cppmicroservices
