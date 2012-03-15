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


#include "usServiceTrackerPrivate.h"
#include "usTrackedService_p.h"
#include "usServiceException.h"
#include "usModuleContext.h"

#include <stdexcept>
#include <limits>

US_BEGIN_NAMESPACE

template<class S, class T>
ServiceTracker<S,T>::~ServiceTracker()
{
  Close();
  delete d;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4355)
#endif

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext* context,
                                    const ServiceReference& reference,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, reference, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext* context, const std::string& clazz,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, clazz, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext* context, const LDAPFilter& filter,
                                    _ServiceTrackerCustomizer* customizer)
  : d(new _ServiceTrackerPrivate(this, context, filter, customizer))
{
}

template<class S, class T>
ServiceTracker<S,T>::ServiceTracker(ModuleContext *context, ServiceTrackerCustomizer<T> *customizer)
  : d(new _ServiceTrackerPrivate(this, context, us_service_interface_iid<S>(), customizer))
{
  const char* clazz = us_service_interface_iid<S>();
  if (clazz == 0) throw ServiceException("The service interface class has no US_DECLARE_SERVICE_INTERFACE macro");
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

template<class S, class T>
void ServiceTracker<S,T>::Open()
{
  _TrackedService* t;
  {
    typename _ServiceTrackerPrivate::Lock l(d);
    if (d->trackedService)
    {
      return;
    }

    US_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::Open: " << d->filter;

    t = new _TrackedService(this, d->customizer);
    {
      typename _TrackedService::Lock l(*t);
      try {
        d->context->AddServiceListener(t, &_TrackedService::ServiceChanged, d->listenerFilter);
        std::list<ServiceReference> references;
        if (!d->trackClass.empty())
        {
          references = d->GetInitialReferences(d->trackClass, std::string());
        }
        else
        {
          if (d->trackReference.GetModule() != 0)
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
        throw std::runtime_error(std::string("unexpected std::invalid_argument exception: ")
            + e.what());
      }
    }
    d->trackedService = t;
  }
  /* Call tracked outside of synchronized region */
  t->TrackInitial(); /* process the initial references */
}

template<class S, class T>
void ServiceTracker<S,T>::Close()
{
  _TrackedService* outgoing;
  std::list<ServiceReference> references;
  {
    typename _ServiceTrackerPrivate::Lock l(d);
    outgoing = d->trackedService;
    if (outgoing == 0)
    {
      return;
    }
    US_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::close:" << d->filter;
    outgoing->Close();
    GetServiceReferences(references);
    d->trackedService = 0;
    try
    {
      d->context->RemoveServiceListener(outgoing, &_TrackedService::ServiceChanged);
    }
    catch (const std::logic_error& /*e*/)
    {
      /* In case the context was stopped. */
    }
  }
  d->Modified(); /* clear the cache */
  {
    typename _TrackedService::Lock l(outgoing);
    outgoing->NotifyAll(); /* wake up any waiters */
  }
  for(std::list<ServiceReference>::const_iterator ref = references.begin();
      ref != references.end(); ++ref)
  {
    outgoing->Untrack(*ref, ServiceEvent());
  }

  if (d->DEBUG)
  {
    typename _ServiceTrackerPrivate::Lock l(d);
    if ((d->cachedReference.GetModule() == 0) && (d->cachedService == 0))
    {
      US_DEBUG(true) << "ServiceTracker<S,T>::close[cached cleared]:"
                       << d->filter;
    }
  }

  delete outgoing;
  d->trackedService = 0;
}

template<class S, class T>
T ServiceTracker<S,T>::WaitForService(unsigned long timeoutMillis)
{
  T object = GetService();
  while (object == 0)
  {
    _TrackedService* t = d->Tracked();
    if (t == 0)
    { /* if ServiceTracker is not open */
      return 0;
    }
    {
      typename _TrackedService::Lock l(t);
      if (t->Size() == 0)
      {
        t->Wait(timeoutMillis);
      }
    }
    object = GetService();
  }
  return object;
}

template<class S, class T>
void ServiceTracker<S,T>::GetServiceReferences(std::list<ServiceReference>& refs) const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return;
  }
  {
    typename _TrackedService::Lock l(t);
    d->GetServiceReferences_unlocked(refs, t);
  }
}

template<class S, class T>
ServiceReference ServiceTracker<S,T>::GetServiceReference() const
{
  ServiceReference reference(0);
  {
    typename _ServiceTrackerPrivate::Lock l(d);
    reference = d->cachedReference;
  }
  if (reference.GetModule() != 0)
  {
    US_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getServiceReference[cached]:"
                         << d->filter;
    return reference;
  }
  US_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getServiceReference:" << d->filter;
  std::list<ServiceReference> references;
  GetServiceReferences(references);
  int length = references.size();
  if (length == 0)
  { /* if no service is being tracked */
    throw ServiceException("No service is being tracked");
  }
  std::list<ServiceReference>::const_iterator selectedRef;
  if (length > 1)
  { /* if more than one service, select highest ranking */
    std::vector<int> rankings(length);
    int count = 0;
    int maxRanking = std::numeric_limits<int>::min();
    std::list<ServiceReference>::const_iterator refIter = references.begin();
    for (int i = 0; i < length; i++)
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
      long int minId = std::numeric_limits<long int>::max();
      refIter = references.begin();
      for (int i = 0; i < length; i++)
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

  {
    typename _ServiceTrackerPrivate::Lock l(d);
    d->cachedReference = *selectedRef;
    return d->cachedReference;
  }
}

template<class S, class T>
T ServiceTracker<S,T>::GetService(const ServiceReference& reference) const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return 0;
  }
  {
    typename _TrackedService::Lock l(t);
    return t->GetCustomizedObject(reference);
  }
}

template<class S, class T>
void ServiceTracker<S,T>::GetServices(std::list<T>& services) const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return;
  }
  {
    typename _TrackedService::Lock l(t);
    std::list<ServiceReference> references;
    d->GetServiceReferences_unlocked(references, t);
    for(std::list<ServiceReference>::const_iterator ref = references.begin();
        ref != references.end(); ++ref)
    {
      services.push_back(t->GetCustomizedObject(*ref));
    }
  }
}

template<class S, class T>
T ServiceTracker<S,T>::GetService() const
{
  T service = d->cachedService;
  if (service != 0)
  {
    US_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getService[cached]:"
                         << d->filter;
    return service;
  }
  US_DEBUG(d->DEBUG) << "ServiceTracker<S,T>::getService:" << d->filter;

  try
  {
    ServiceReference reference = GetServiceReference();
    if (reference.GetModule() == 0)
    {
      return 0;
    }
    return d->cachedService = GetService(reference);
  }
  catch (const ServiceException&)
  {
    return 0;
  }
}

template<class S, class T>
void ServiceTracker<S,T>::Remove(const ServiceReference& reference)
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return;
  }
  t->Untrack(reference, ServiceEvent());
}

template<class S, class T>
int ServiceTracker<S,T>::Size() const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return 0;
  }
  {
    typename _TrackedService::Lock l(t);
    return t->Size();
  }
}

template<class S, class T>
int ServiceTracker<S,T>::GetTrackingCount() const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return -1;
  }
  {
    typename _TrackedService::Lock l(t);
    return t->GetTrackingCount();
  }
}

template<class S, class T>
void ServiceTracker<S,T>::GetTracked(TrackingMap& map) const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return;
  }
  {
    typename _TrackedService::Lock l(t);
    t->CopyEntries(map);
  }
}

template<class S, class T>
bool ServiceTracker<S,T>::IsEmpty() const
{
  _TrackedService* t = d->Tracked();
  if (t == 0)
  { /* if ServiceTracker is not open */
    return true;
  }
  {
    typename _TrackedService::Lock l(t);
    return t->IsEmpty();
  }
}

template<class S, class T>
T ServiceTracker<S,T>::AddingService(const ServiceReference& reference)
{
 return dynamic_cast<T>(d->context->GetService(reference));
}

template<class S, class T>
void ServiceTracker<S,T>::ModifiedService(const ServiceReference& /*reference*/, T /*service*/)
{
  /* do nothing */
}

template<class S, class T>
void ServiceTracker<S,T>::RemovedService(const ServiceReference& reference, T /*service*/)
{
  d->context->UngetService(reference);
}

US_END_NAMESPACE
