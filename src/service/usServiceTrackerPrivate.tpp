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


#include "usTrackedService_p.h"

#include "usModuleContext.h"
#include "usLDAPFilter.h"

#include <stdexcept>

US_BEGIN_NAMESPACE

template<class S, class T>
const bool ServiceTrackerPrivate<S,T>::DEBUG = true;

template<class S, class T>
ServiceTrackerPrivate<S,T>::ServiceTrackerPrivate(
    ServiceTracker<S,T>* st, ModuleContext* context,
    const ServiceReference& reference,
    ServiceTrackerCustomizer<T>* customizer)
  : context(context), customizer(customizer), trackReference(reference),
    trackedService(0), cachedReference(0), cachedService(0), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  std::stringstream ss;
  ss << "(" << ServiceConstants::SERVICE_ID() << "="
     << any_cast<long>(reference.GetProperty(ServiceConstants::SERVICE_ID())) << ")";
  this->listenerFilter = ss.str();
  try
  {
    this->filter = LDAPFilter(listenerFilter);
  }
  catch (const std::invalid_argument& e)
  {
    /*
     * we could only get this exception if the ServiceReference was
     * invalid
     */
    std::invalid_argument ia(std::string("unexpected std::invalid_argument exception: ") + e.what());
    throw ia;
  }
}

template<class S, class T>
ServiceTrackerPrivate<S,T>::ServiceTrackerPrivate(
    ServiceTracker<S,T>* st,
    ModuleContext* context, const std::string& clazz,
    ServiceTrackerCustomizer<T>* customizer)
      : context(context), customizer(customizer), trackClass(clazz),
        trackReference(0), trackedService(0), cachedReference(0),
        cachedService(0), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  this->listenerFilter = std::string("(") + US_PREPEND_NAMESPACE(ServiceConstants)::OBJECTCLASS() + "="
                        + clazz + ")";
  try
  {
    this->filter = LDAPFilter(listenerFilter);
  }
  catch (const std::invalid_argument& e)
  {
    /*
     * we could only get this exception if the clazz argument was
     * malformed
     */
    std::invalid_argument ia(
        std::string("unexpected std::invalid_argument exception: ") + e.what());
    throw ia;
  }
}

template<class S, class T>
ServiceTrackerPrivate<S,T>::ServiceTrackerPrivate(
    ServiceTracker<S,T>* st,
    ModuleContext* context, const LDAPFilter& filter,
    ServiceTrackerCustomizer<T>* customizer)
      : context(context), filter(filter), customizer(customizer),
        listenerFilter(filter.ToString()), trackReference(0),
        trackedService(0), cachedReference(0), cachedService(0), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  if (context == 0)
  {
    throw std::invalid_argument("The module context cannot be null.");
  }
}

template<class S, class T>
ServiceTrackerPrivate<S,T>::~ServiceTrackerPrivate()
{

}

template<class S, class T>
std::list<ServiceReference> ServiceTrackerPrivate<S,T>::GetInitialReferences(
  const std::string& className, const std::string& filterString)
{
  return context->GetServiceReferences(className, filterString);
}

template<class S, class T>
void ServiceTrackerPrivate<S,T>::GetServiceReferences_unlocked(std::list<ServiceReference>& refs, TrackedService<S,T>* t) const
{
  if (t->Size() == 0)
  {
    return;
  }
  t->GetTracked(refs);
}

template<class S, class T>
TrackedService<S,T>* ServiceTrackerPrivate<S,T>::Tracked() const
{
  return trackedService;
}

template<class S, class T>
void ServiceTrackerPrivate<S,T>::Modified()
{
  cachedReference = 0; /* clear cached value */
  cachedService = 0; /* clear cached value */
  US_DEBUG(DEBUG) << "ServiceTracker::Modified(): " << filter;
}

US_END_NAMESPACE
