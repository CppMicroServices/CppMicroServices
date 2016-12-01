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

#ifndef CPPMICROSERVICES_BUNDLETRACKER_H
#define CPPMICROSERVICES_BUNDLETRACKER_H

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/BundleTrackerCustomizer.h>

#include <memory>

class BundleTrackerPrivate;

namespace cppmicroservices {

/**
 * The BundleTracker class simplifies tracking bundles much like the ServiceTracker simplifies tracking 
 * services.
 * 
 * <p>
 * A BundleTracker is constructed with state criteria and a BundleTrackerCustomizer object. A BundleTracker
 * can use the BundleTrackerCustomizer to select which bundles are tracked and to create a
 * customized object to be tracked with the bundle. The BundleTracker can then be opened to begin
 * tracking all bundles whose state matches the specified state criteria.
 *
 * <p>
 * The getBundles method can be called to get the Bundle objects of the bundles being tracked. The
 * getObject method can be called to get the customized object for a tracked bundle.
 *
 * <p>
 * The BundleTracker class is thread-safe. It does not call a BundleTrackerCustomizer while holding * any locks. BundleTrackerCustomizer implementations must also be thread-safe. * 
 * \tparam T The type of the tracked object.
 * \remarks This class is thread safe.
 */
template <class T>
class BundleTracker<T> : protected BundleTrackerCustomizer<T>
{
public:

  /**
   * Create a BundleTracker for bundles whose state is present in the specified state mask.
   * Bundles whose state is present on the specified state mask will be tracked by this BundleTracker.
   *
   * @param context The \c BundleContext against which the tracking is done.
   * @param bundleStateMask The bit mask of the bundle states to be tracked.
   * @param customizer The customizer object to call when bundles are added, modified, or removed in this BundleTracker.
   *                   If customizer is nullptr, then this BundleTracker will be used as the BundleTrackerCustomizer and this
   *                   BundleTracker will call the BundleTrackerCustomizer methods on itself.
   *
   * @see Bundle::GetState()
   */
  BundleTracker(const BundleContext& context, uint32_t bundleStateMask, BundleTrackerCustomizer<T>* customizer = nullptr) {}
  virtual ~BundleTracker() {}

 /**
  * A bundle is being added to the BundleTracker.
  * Default implementation of the BundleTrackerCustomizer::AddingBundle method.
  * This method is only called when this BundleTracker has been constructed with a nullptr BundleTrackerCustomizer
  * argument.
  *
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
  *
  * @see BundleTrackerCustomizer::AddingBundle(Bundle, BundleEvent)
  */
  std::shared_ptr<T> AddingBundle(const Bundle& bundle, const BundleEvent& event);

  /**
   * Close this BundleTracker.
   *
   * This method should be called when this BundleTracker should end the tracking of bundles.
   * This implementation calls GetBundles() to get the list of tracked bundles to remove.
   */
  void Close();
       
  /**
   * Return an array of Bundles for all bundles being tracked by this BundleTracker.
   *
   * @return A vector of Bundles or an empty vector if no bundles are being tracked.
   */
  std::vector<Bundle> GetBundles();

  /**
   * Returns the customized object for the specified Bundle if the specified bundle is being tracked by
   * this BundleTracker.
   *
   * @param bundle The Bundle being tracked.
   * @return The customized object for the specified Bundle or null if the specified Bundle is not being tracked.
   */   
  std::shared_ptr<T> GetObject(const Bundle& bundle);
        
  /**
   * Return a Map with the Bundles and customized objects for all bundles being tracked by this BundleTracker.
   *
   * @return A map with the Bundles and customized objects for all services being tracked by this BundleTracker. 
   *         If no bundles are being tracked, then the returned map is empty.
   */
  std::map<Bundle, std::shared_ptr<T>> GetTracked();

  /**
   * Returns the tracking count for this BundleTracker.
   *
   * The tracking count is initialized to 0 when this BundleTracker is opened.
   * Every time a bundle is added, modified or removed from this BundleTracker the tracking count is incremented.
   * The tracking count can be used to determine if this BundleTracker has added, modified or removed
   * a bundle by comparing a tracking count value previously collected with the current tracking count value.
   * 
   * If the value has not changed, then no bundle has been added, modified or removed from this
   * BundleTracker since the previous tracking count was collected.
   *
   * @return The tracking count for this BundleTracker or -1 if this BundleTracker is not open.
   */
  int GetTrackingCount();

  /**
   * Return if this BundleTracker is empty.
   * @return true if this BundleTracker is not tracking any bundles.
   */
  bool IsEmpty();

  /**
   * A bundle tracked by the BundleTracker has been modified.
   * Default implementation of the BundleTrackerCustomizer::ModifiedBundle method.
   * This method is only called when this BundleTracker has been constructed with a nullptr BundleTrackerCustomizer
   * argument.
   *
   * This method is called when a bundle being tracked by the BundleTracker has had its state modified.
   *
   * @param bundle The Bundle whose state has been modified.
   * @param event The /c BundleEvent which caused this customizer method to be called or an invalid /c BundleEvent
   *              if there is no bundle event associated with the call to this method.
   * @tparam object The tracked object for the specified bundle.
   *
   * @see BundleTrackerCustomizer::ModifiedBundle(Bundle, BundleEvent, object)
   */
  void ModifiedBundle(const Bundle& bundle, const BundleEvent& event, std::shared_ptr<T> object);

  /**
   * Open this BundleTracker and begin tracking bundles.
   *
   * Bundle which match the state criteria specified when this BundleTracker was created are now
   * tracked by this BundleTracker.
   *
   * @throws std::logic_error If the BundleContext with which this BundleTracker was created is no
   *         longer valid.
   */
  void Open();
  
  /**
   * Remove a bundle from this BundleTracker.
   *
   * The specified bundle will be removed from this BundleTracker.
   * If the specified bundle was being tracked then the BundleTrackerCustomizer::RemovedBundle method 
   * will be called for that bundle.
   *
   * @param bundle The Bundle to be removed.
   */
  void Remove(const Bundle& bundle);

  /**
   * A bundle tracked by the BundleTracker has been removed.
   * Default implementation of the BundleTrackerCustomizer::RemovedBundle method.
   * This method is only called when this BundleTracker has been constructed with a nullptr BundleTrackerCustomizer
   * argument.
   *
   * This method is called after a bundle is no longer being tracked by the BundleTracker.
   *
   * @param bundle The Bundle that has been removed.
   * @param event The /c BundleEvent which caused this customizer method to be called or an invalid /c BundleEvent
   *              if there is no bundle event associated with the call to this method.
   * @tparam object The tracked object for the specified bundle.
   *
   * @see BundleTrackerCustomizer::RemovedBundle(Bundle, BundleEvent, object)
   */
  virtual void RemovedBundle(const Bundle& bundle, const BundleEvent& event, std::shared_ptr<T> object);

  /**
   * Return the number of bundles being tracked by this BundleTracker.
   *
   * @return The number of bundles being tracked.
   */
  int Size();

private:
    std::unique_ptr<BundleTrackerPrivate> d;
};

}


#endif // CPPMICROSERVICES_BUNDLETRACKER_H
