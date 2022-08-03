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

template<class TTT>
TrackedBundle<TTT>::TrackedBundle(BundleTracker<T>* bundleTracker,
                                  BundleTrackerCustomizer<T>* customizer)
  : Superclass(bundleTracker->d->context)
  , bundleTracker(bundleTracker)
  , customizer(customizer)
  , latch{}
{
}

template<class TTT>
void TrackedBundle<TTT>::WaitOnCustomizersToFinish()
{
  latch.Wait();
}

template<class TTT>
void TrackedBundle<TTT>::BundleChanged(const BundleEvent& event)
{
  // Call track or untrack based on state mask

  // Ignore events that do not correspond with
  // Bundle state changes

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
  BundleEvent::Type eventType = event.GetType();
  if (eventType == BundleEvent::Type::BUNDLE_UNRESOLVED)
  {
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
    this->Track(bundle, event);
    /*
     * If the customizer throws an unchecked exception, it is
     * safe to let it propagate
     */
  } else {
    this->Untrack(bundle, event);
    /*
     * If the customizer throws an unchecked exception, it is
     * safe to let it propagate
     */
  }
}

template<class TTT>
void TrackedBundle<TTT>::Modified()
{
  Superclass::Modified(); /* increment the modification count */
  bundleTracker->d->Modified();
}

template<class TTT>
std::optional<typename TrackedBundle<TTT>::TrackedParamType>
TrackedBundle<TTT>::CustomizerAdding(Bundle bundle, const BundleEvent& event)
{
  return customizer->AddingBundle(bundle, event);
}

template<class TTT>
void TrackedBundle<TTT>::CustomizerModified(Bundle bundle,
                                            const BundleEvent& event,
                                            const TrackedParamType& object)
{
  customizer->ModifiedBundle(bundle, event, object);
}

template<class TTT>
void TrackedBundle<TTT>::CustomizerRemoved(Bundle bundle,
                                           const BundleEvent& event,
                                           const TrackedParamType& object)
{
  customizer->RemovedBundle(bundle, event, object);
}

} // namespace detail

} // namespace cppmicroservices
