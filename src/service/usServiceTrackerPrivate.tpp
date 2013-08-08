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

template<class S, class TTT>
const bool ServiceTrackerPrivate<S,TTT>::DEBUG_OUTPUT = true;

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::ServiceTrackerPrivate(
    ServiceTracker<S,TTT>* st, ModuleContext* context,
    const ServiceReference<S>& reference,
    ServiceTrackerCustomizer<S,T>* customizer)
  : context(context), customizer(customizer), trackReference(reference),
    trackedService(0), cachedReference(), cachedService(TTT::DefaultValue()), q_ptr(st)
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

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::ServiceTrackerPrivate(
    ServiceTracker<S,TTT>* st,
    ModuleContext* context, const std::string& clazz,
    ServiceTrackerCustomizer<S,T>* customizer)
      : context(context), customizer(customizer), trackClass(clazz),
        trackReference(), trackedService(0), cachedReference(),
        cachedService(TTT::DefaultValue()), q_ptr(st)
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

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::ServiceTrackerPrivate(
    ServiceTracker<S,TTT>* st,
    ModuleContext* context, const LDAPFilter& filter,
    ServiceTrackerCustomizer<S,T>* customizer)
      : context(context), filter(filter), customizer(customizer),
        listenerFilter(filter.ToString()), trackReference(),
        trackedService(0), cachedReference(), cachedService(TTT::DefaultValue()), q_ptr(st)
{
  this->customizer = customizer ? customizer : q_func();
  if (context == 0)
  {
    throw std::invalid_argument("The module context cannot be null.");
  }
}

template<class S, class TTT>
ServiceTrackerPrivate<S,TTT>::~ServiceTrackerPrivate()
{

}

template<class S, class TTT>
std::vector<ServiceReference<S> > ServiceTrackerPrivate<S,TTT>::GetInitialReferences(
  const std::string& className, const std::string& filterString)
{
  std::vector<ServiceReference<S> > result;
  std::vector<ServiceReferenceU> refs = context->GetServiceReferences(className, filterString);
  for(std::vector<ServiceReferenceU>::const_iterator iter = refs.begin();
      iter != refs.end(); ++iter)
  {
    ServiceReference<S> ref(*iter);
    if (ref)
    {
      result.push_back(ref);
    }
  }
  return result;
}

template<class S, class TTT>
void ServiceTrackerPrivate<S,TTT>::GetServiceReferences_unlocked(std::vector<ServiceReference<S> >& refs, TrackedService<S,TTT>* t) const
{
  if (t->Size() == 0)
  {
    return;
  }
  t->GetTracked(refs);
}

template<class S, class TTT>
TrackedService<S,TTT>* ServiceTrackerPrivate<S,TTT>::Tracked() const
{
  return trackedService;
}

template<class S, class TTT>
void ServiceTrackerPrivate<S,TTT>::Modified()
{
  cachedReference = 0; /* clear cached value */
  TTT::Dispose(cachedService); /* clear cached value */
  US_DEBUG(DEBUG_OUTPUT) << "ServiceTracker::Modified(): " << filter;
}

US_END_NAMESPACE
