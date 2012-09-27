/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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


#ifndef USSERVICELISTENERS_H
#define USSERVICELISTENERS_H

#include <list>
#include <string>
#include <set>

#include <usConfig.h>

#include "usServiceListenerEntry_p.h"

US_BEGIN_NAMESPACE

class CoreModuleContext;
class ModuleContext;

/**
 * Here we handle all listeners that modules have registered.
 *
 */
class ServiceListeners {

private:

  typedef Mutex MutexType;
  typedef MutexLock<MutexType> MutexLocker;

public:

  typedef US_MODULE_LISTENER_FUNCTOR ModuleListener;
  typedef US_UNORDERED_MAP_TYPE<ModuleContext*, std::list<std::pair<ModuleListener,void*> > > ModuleListenerMap;
  ModuleListenerMap moduleListenerMap;
  MutexType moduleListenerMapMutex;

  typedef US_UNORDERED_MAP_TYPE<std::string, std::list<ServiceListenerEntry> > CacheType;
  typedef US_UNORDERED_SET_TYPE<ServiceListenerEntry> ServiceListenerEntries;

private:

  std::vector<std::string> hashedServiceKeys;
  static const int OBJECTCLASS_IX; // = 0;
  static const int SERVICE_ID_IX; // = 1;

  /* Service listeners with complicated or empty filters */
  std::list<ServiceListenerEntry> complicatedListeners;

  /* Service listeners with "simple" filters are cached. */
  CacheType cache[2];

  ServiceListenerEntries serviceSet;

  MutexType mutex;

public:

  ServiceListeners();

  /**
   * Add a new service listener. If an old one exists, and it has the
   * same owning module, the old listener is removed first.
   *
   * @param mc The module context adding this listener.
   * @param listener The service listener to add.
   * @param data Additional data to distinguish ServiceListener objects.
   * @param filter An LDAP filter string to check when a service is modified.
   * @exception org.osgi.framework.InvalidSyntaxException
   * If the filter is not a correct LDAP expression.
   */
  void AddServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                          void* data, const std::string& filter);

  /**
   * Remove service listener from current framework. Silently ignore
   * if listener doesn't exist. If listener is registered more than
   * once remove all instances.
   *
   * @param mc The module context who wants to remove listener.
   * @param listener Object to remove.
   * @param data Additional data to distinguish ServiceListener objects.
   */
  void RemoveServiceListener(ModuleContext* mc, const ServiceListenerEntry::ServiceListener& listener,
                             void* data);

  /**
   * Add a new module listener.
   *
   * @param mc The module context adding this listener.
   * @param listener The module listener to add.
   * @param data Additional data to distinguish ModuleListener objects.
   */
  void AddModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data);

  /**
   * Remove module listener from current framework. Silently ignore
   * if listener doesn't exist.
   *
   * @param mc The module context who wants to remove listener.
   * @param listener Object to remove.
   * @param data Additional data to distinguish ModuleListener objects.
   */
  void RemoveModuleListener(ModuleContext* mc, const ModuleListener& listener, void* data);

  void ModuleChanged(const ModuleEvent& evt);

  /**
   * Remove all listener registered by a module in the current framework.
   *
   * @param mc Module context which listeners we want to remove.
   */
  void RemoveAllListeners(ModuleContext* mc);

  /**
   * Receive notification that a service has had a change occur in its lifecycle.
   *
   * @see org.osgi.framework.ServiceListener#serviceChanged
   */
  void ServiceChanged(const ServiceListenerEntries& receivers,
                      const ServiceEvent& evt,
                      ServiceListenerEntries& matchBefore);

  void ServiceChanged(const ServiceListenerEntries& receivers,
                      const ServiceEvent& evt);

  /**
   *
   *
   */
  void GetMatchingServiceListeners(const ServiceReference& sr, ServiceListenerEntries& listeners,
                                   bool lockProps = true);


private:

  void RemoveServiceListener_unlocked(const ServiceListenerEntry& entryToRemove);

  /**
   * Remove all references to a service listener from the service listener
   * cache.
   */
  void RemoveFromCache(const ServiceListenerEntry& sle);

  /**
   * Checks if the specified service listener's filter is simple enough
   * to cache.
   */
  void CheckSimple(const ServiceListenerEntry& sle);

  void AddToSet(ServiceListenerEntries& set, int cache_ix, const std::string& val);

};

US_END_NAMESPACE

#endif // USSERVICELISTENERS_H
