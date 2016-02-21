/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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


#include "usServiceTrackerPrivate.h"
#include "usTrackedService_p.h"
#include "usServiceException.h"
#include "usBundleContext.h"

#include <string>
#include <stdexcept>
#include <limits>

namespace us {

template<class S, class T>
ServiceTracker<S,T>::~ServiceTracker()
{
  Close();
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355)
#endif

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(BundleContext* context,
                                    const ServiceReference<S>& reference,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, reference, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(BundleContext* context, const std::string& clazz,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, clazz, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(BundleContext* context, const LDAPFilter& filter,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, filter, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(BundleContext *context, _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, us_service_interface_iid<S>(), customizer))
{
  std::string clazz = us_service_interface_iid<S>();
  if (clazz.empty()) throw ServiceException("The service interface class has no US_DECLARE_SERVICE_INTERFACE macro");
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

template<class S, class T>
void ServiceTracker<S,T>::Open()
{
  std::unique_ptr<_TrackedService> t;
  {
    auto l = d->Lock(); US_UNUSED(l);
    if (d->trackedService)
    {
      return;
    }

    US_DEBUG(d->DEBUG_OUTPUT) << "ServiceTracker<S,TTT>::Open: " << d->filter;

    t.reset(new _TrackedService(this, d->customizer));
    try
    {
      d->context->AddServiceListener(t.get(), &_TrackedService::ServiceChanged, d->listenerFilter);
      std::vector<ServiceReference<S>> references;
      if (!d->trackClass.empty())
      {
        references = d->GetInitialReferences(d->trackClass, std::string());
      }
      else
      {
        if (d->trackReference.GetBundle() != nullptr)
        {
          references.push_back(d->trackReference);
        }
        else
        { /* user supplied filter */
          references = d->GetInitialReferences(std::string(),
                                               (d->listenerFilter.empty()) ? d->filter.ToString() : d->listenerFilter);
        }
      }
      /* set tracked with the initial references */
      t->SetInitial(references);
    }
    catch (const std::invalid_argument& e)
    {
      d->context->RemoveServiceListener(t.get(), &_TrackedService::ServiceChanged);
      throw std::runtime_error(std::string("unexpected std::invalid_argument exception: ")
                               + e.what());
    }
    d->trackedService = std::move(t);
  }
  /* Call tracked outside of synchronized region */
  d->trackedService->TrackInitial(); /* process the initial references */
}

template<class S, class T>
void ServiceTracker<S,T>::Close()
{
  std::unique_ptr<_TrackedService> outgoing;
  std::vector<ServiceReference<S>> references;
  {
    auto l = d->Lock(); US_UNUSED(l);
    outgoing = std::move(d->trackedService);
    d->trackedService.reset();
    if (outgoing == nullptr)
    {
      return;
    }
    US_DEBUG(d->DEBUG_OUTPUT) << "ServiceTracker<S,TTT>::close:" << d->filter;
    outgoing->Close();
    references = GetServiceReferences();
    try
    {
      d->context->RemoveServiceListener(outgoing.get(), &_TrackedService::ServiceChanged);
    }
    catch (const std::logic_error& /*e*/)
    {
      /* In case the context was stopped. */
    }
  }
  d->Modified(); /* clear the cache */
  outgoing->NotifyAll(); /* wake up any waiters */
  for(auto& ref : references)
  {
    outgoing->Untrack(ref, ServiceEvent());
  }

  if (d->DEBUG_OUTPUT)
  {
    if (d->cachedReference.Load().GetBundle() == nullptr &&
        std::atomic_load(&d->cachedService) == nullptr)
    {
      US_DEBUG(true) << "ServiceTracker<S,TTT>::close[cached cleared]:"
                       << d->filter;
    }
  }
}

template<class S, class T>
std::shared_ptr<typename ServiceTracker<S,T>::TrackedParmType>
ServiceTracker<S,T>::WaitForService()
{
  auto object = GetService();
  if (!object)
  {
    _TrackedService* t = d->Tracked();
    if (t == nullptr)
    { /* if ServiceTracker is not open */
      return object;
    }
    {
      auto l = t->Lock();
      if (t->Size_unlocked() == 0)
      {
        t->Wait(l, [&t]{ return t->Size_unlocked(); });
      }
    }
    object = GetService();
  }
  return object;
}

template<class S, class T>
template<class Rep, class Period>
std::shared_ptr<typename ServiceTracker<S,T>::TrackedParmType>
ServiceTracker<S,T>::WaitForService(const std::chrono::duration<Rep, Period>& rel_time)
{
  auto object = GetService();
  while (!object)
  {
    _TrackedService* t = d->Tracked();
    if (t == nullptr)
    { /* if ServiceTracker is not open */
      return std::shared_ptr<TrackedParmType>();
    }
    {
      auto l = t->Lock();
      if (t->Size_unlocked() == 0)
      {
        t->WaitFor(l, rel_time);
      }
    }
    object = GetService();
  }
  return object;
}

template<class S, class T>
std::vector<ServiceReference<S>>
ServiceTracker<S,T>::GetServiceReferences() const
{
  std::vector<ServiceReference<S>> refs;
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return refs;
  }
  {
    auto l = t->Lock(); US_UNUSED(l);
    d->GetServiceReferences_unlocked(refs, t);
  }
  return refs;
}

template<class S, class T>
ServiceReference<S>
ServiceTracker<S,T>::GetServiceReference() const
{
  ServiceReference<S> reference = d->cachedReference.Load();
  if (reference.GetBundle() != nullptr)
  {
    US_DEBUG(d->DEBUG_OUTPUT) << "ServiceTracker<S,TTT>::getServiceReference[cached]:"
                         << d->filter;
    return reference;
  }
  US_DEBUG(d->DEBUG_OUTPUT) << "ServiceTracker<S,TTT>::getServiceReference:" << d->filter;
  auto references = GetServiceReferences();
  std::size_t length = references.size();
  if (length == 0)
  { /* if no service is being tracked */
    throw ServiceException("No service is being tracked");
  }
  auto selectedRef = references.begin();
  if (length > 1)
  { /* if more than one service, select highest ranking */
    std::vector<int> rankings(length);
    int count = 0;
    int maxRanking = (std::numeric_limits<int>::min)();
    auto refIter = references.begin();
    for (std::size_t i = 0; i < length; i++)
    {
      Any rankingAny = refIter->GetProperty(ServiceConstants::SERVICE_RANKING());
      int ranking = 0;
      if (rankingAny.Type() == typeid(int))
      {
        ranking = any_cast<int>(rankingAny);
      }

      rankings[i] = ranking;
      if (ranking > maxRanking)
      {
        selectedRef = refIter;
        maxRanking = ranking;
        count = 1;
      }
      else
      {
        if (ranking == maxRanking)
        {
          count++;
        }
      }
      ++refIter;
    }
    if (count > 1)
    { /* if still more than one service, select lowest id */
      long int minId = (std::numeric_limits<long int>::max)();
      refIter = references.begin();
      for (std::size_t i = 0; i < length; i++)
      {
        if (rankings[i] == maxRanking)
        {
          Any idAny = refIter->GetProperty(ServiceConstants::SERVICE_ID());
          long int id = 0;
          if (idAny.Type() == typeid(long int))
          {
            id = any_cast<long int>(idAny);
          }
          if (id < minId)
          {
            selectedRef = refIter;
            minId = id;
          }
        }
        ++refIter;
      }
    }
  }

  d->cachedReference.Store(*selectedRef);
  return *selectedRef;
}

template<class S, class T>
std::shared_ptr<typename ServiceTracker<S,T>::TrackedParmType>
ServiceTracker<S,T>::GetService(const ServiceReference<S>& reference) const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return std::shared_ptr<TrackedParmType>();
  }
  return (t->Lock(), t->GetCustomizedObject_unlocked(reference));
}

template<class S, class T>
std::vector<std::shared_ptr<typename ServiceTracker<S,T>::TrackedParmType>> ServiceTracker<S,T>::GetServices() const
{
  std::vector<std::shared_ptr<TrackedParmType>> services;
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return services;
  }
  {
    auto l = t->Lock(); US_UNUSED(l);
    std::vector<ServiceReference<S>> references;
    d->GetServiceReferences_unlocked(references, t);
    for(auto& ref : references)
    {
      services.push_back(t->GetCustomizedObject_unlocked(ref));
    }
  }
  return services;
}

template<class S, class T>
std::shared_ptr<typename ServiceTracker<S,T>::TrackedParmType>
ServiceTracker<S,T>::GetService() const
{
  auto service = std::atomic_load(&d->cachedService);
  if (service)
  {
    US_DEBUG(d->DEBUG_OUTPUT) << "ServiceTracker<S,TTT>::getService[cached]:"
                              << d->filter;
    return service;
  }
  US_DEBUG(d->DEBUG_OUTPUT) << "ServiceTracker<S,TTT>::getService:" << d->filter;

  try
  {
    auto reference = GetServiceReference();
    if (reference.GetBundle() == nullptr)
    {
      return std::shared_ptr<TrackedParmType>();
    }
    service = GetService(reference);
    std::atomic_store(&d->cachedService, service);
    return service;
  }
  catch (const ServiceException&)
  {
    return std::shared_ptr<TrackedParmType>();
  }
}

template<class S, class T>
void ServiceTracker<S,T>::Remove(const ServiceReference<S>& reference)
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return;
  }
  t->Untrack(reference, ServiceEvent());
}

template<class S, class T>
int ServiceTracker<S,T>::Size() const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return 0;
  }
  return (t->Lock(), static_cast<int>(t->Size_unlocked()));
}

template<class S, class T>
int ServiceTracker<S,T>::GetTrackingCount() const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return -1;
  }
  return (t->Lock(), t->GetTrackingCount());
}

template<class S, class T>
void ServiceTracker<S,T>::GetTracked(TrackingMap& map) const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return;
  }
  t->Lock(), t->CopyEntries_unlocked(map);
}

template<class S, class T>
bool ServiceTracker<S,T>::IsEmpty() const
{
  _TrackedService* t = d->Tracked();
  if (t == nullptr)
  { /* if ServiceTracker is not open */
    return true;
  }
  return (t->Lock(), t->IsEmpty_unlocked());
}

template<class S, class T>
std::shared_ptr<typename ServiceTracker<S,T>::TrackedParmType>
ServiceTracker<S,T>::AddingService(const ServiceReference<S>& reference)
{
  return TypeTraits::ConvertToTrackedType(d->context->GetService(reference));
}

template<class S, class T>
void ServiceTracker<S,T>::ModifiedService(const ServiceReference<S>& /*reference*/, const std::shared_ptr<TrackedParmType>& /*service*/)
{
  /* do nothing */
}

template<class S, class T>
void ServiceTracker<S,T>::RemovedService(const ServiceReference<S>& /*reference*/, const std::shared_ptr<TrackedParmType>& /*service*/)
{
  /* do nothing */
}

}
