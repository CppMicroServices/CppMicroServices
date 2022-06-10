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

#include <vector>

namespace cppmicroservices {

// Destructor
template<class T>
BundleTracker<T>::~BundleTracker()
{
  try
  {
    Close();
  }
  catch (...) {}
}

template<class T>
BundleTracker<T>::BundleTracker(const BundleContext& context,
                                uint32_t stateMask,
                                std::unique_ptr<_BundleTrackerCustomizer> customizer)
  : d(new _BundleTrackerPrivate(this, context, stateMask, customizer))
{
}

template<class T>
void BundleTracker<T>::Open()
{
  // TODO: BT Open
}

template<class T>
void BundleTracker<T>::Close()
{
  // TODO: BT Close
}

template<class T>
std::vector<Bundle> BundleTracker<T>::GetBundles()
{
  // TODO: BT GetBundles
  return NULL;
}

template<class T>
std::shared_ptr<typename BundleTracker<T>::TrackedParamType> 
BundleTracker<T>::GetObject(const Bundle& bundle)
{
  // TODO: BT GetObject
  return nullptr;
}

template<class T>
typename BundleTracker<T>::TrackingMap
BundleTracker<T>::GetTracked()
{
  // TODO: BT GetTracked
  return NULL;
}

template<class T>
size_t BundleTracker<T>::GetTrackingCount()
{
  // TODO: BT GetTrackingCound
  return 0;
}

template<class T>
bool BundleTracker<T>::IsEmpty()
{
  // TODO: BT IsEmpty
  return false;
}

template<class T>
void BundleTracker<T>::Remove(const Bundle&)
{
  // TODO: BT Remove
}

template<class T>
size_t BundleTracker<T>::Size()
{
  // TODO: BT Size
}

template<class T>
std::shared_ptr<typename BundleTracker<T>::TrackedParamType>
BundleTracker<T>::AddingBundle(const Bundle& bundle,
                               const BundleEvent& event)
{
  // TODO: BT AddingBundle
  return nullptr;
}

template<class T>
void BundleTracker<T>::ModifiedBundle(const Bundle& bundle,
                                      const BundleEvent& event,
                                      std::shared_ptr<typename BundleTracker<T>::TrackedParamType> object)
{
  // no-op 
}

template<class T>
void BundleTracker<T>::RemovedBundle(const Bundle& bundle,
                                     const BundleEvent& event,
                                     std::shared_ptr<typename BundleTracker<T>::TrackedParamType> object)
{
  // no-op 
}

}