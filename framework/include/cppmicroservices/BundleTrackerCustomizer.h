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

#ifndef CPPMICROSERVICES_BUNDLETRACKERCUSTOMIZER_H
#define CPPMICROSERVICES_BUNDLETRACKERCUSTOMIZER_H

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleEvent.h>

#include <memory>

namespace cppmicroservices {

/**
 * The BundleTrackerCustomizer interface allows a BundleTracker to customize the Bundles that are
 * tracked. A BundleTrackerCustomizer is called when a bundle is being added to a BundleTracker. The
 * BundleTrackerCustomizer can then return an object for the tracked bundle. A BundleTrackerCustomizer
 * is also called when a tracked bundle is modified or has been removed from a BundleTracker.
 * 
 * <p>
 * The methods in this interface may be called as the result of a BundleEvent being received by a
 * BundleTracker. Since BundleEvents are received synchronously by the BundleTracker, it is highly
 * recommended that implementations of these methods do not alter bundle states while being synchronized
 * on any object.
 * 
 * <p>
 * The BundleTracker class is thread-safe. It does not call a BundleTrackerCustomizer while holding
 * any locks. BundleTrackerCustomizer implementations must also be thread-safe.
 *
 * \tparam T The type of the tracked object. The default is \c Bundle.
 * \remarks This class is thread safe.
 */
template <class T>
class BundleTrackerCustomizer<T>
{
public:
  /**
   * A bundle is being added to the BundleTracker.
   * This method is called before a bundle which matched the search parameters of the BundleTracker
   * is added to the BundleTracker. This method should return the object to be tracked for the specified
   * Bundle. The returned object is stored in the BundleTracker and is available from the getObject
   * method.
   *
   * @param bundle The /c Bundle being added to the BundleTracker.
   * @param event The /c BundleEvent which caused this customizer method to be called or an invalid /c BundleEvent 
   *              if there is no bundle event associated with the call to this method.
   * @return The object to be tracked for the specified \c Bundle object or nullptr if the specified Bundle object should 
   *         not be tracked.
   */
  virtual std::shared_ptr<T> AddingBundle(const Bundle& bundle, const BundleEvent& event) = 0;

  /**
   * A bundle tracked by the BundleTracker has been modified.
   * This method is called when a bundle being tracked by the BundleTracker has had its state modified.
   *
   * @param bundle The Bundle whose state has been modified.
   * @param event The /c BundleEvent which caused this customizer method to be called or an invalid /c BundleEvent 
   *              if there is no bundle event associated with the call to this method.
   * @tparam object The tracked object for the specified bundle.
   */
  virtual void ModifiedBundle(const Bundle& bundle, const BundleEvent& event, std::shared_ptr<T> object) = 0;

  /**
   * A bundle tracked by the BundleTracker has been removed.
   * This method is called after a bundle is no longer being tracked by the BundleTracker.
   *
   * @param bundle The Bundle that has been removed.
   * @param event The /c BundleEvent which caused this customizer method to be called or an invalid /c BundleEvent 
   *              if there is no bundle event associated with the call to this method.
   * @tparam object The tracked object for the specified bundle.
   */
  virtual void RemovedBundle(const Bundle& bundle, const BundleEvent& event, std::shared_ptr<T> object) = 0;

  virtual ~BundleTrackerCustomizer() {}
};

}

#endif // CPPMICROSERVICES_BUNDLETRACKERCUSTOMIZER_H
