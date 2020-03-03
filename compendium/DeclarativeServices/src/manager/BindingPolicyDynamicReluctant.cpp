/**
 * @file      BindingPolicyDynamicReluctant.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant::ServiceAdded(const ServiceReferenceBase& reference)
{
  // check cardinality... if it's 1..1 or 0..1, do the same thing as StaticReluctant
  // else...
  //   update bound refs,
  //   perform bind/unbind callbacks
  std::vector<RefChangeNotification> notifications;
  if (1 == mgr.metadata.maxCardinality)
  {
    notifications = ReluctantServiceAdded(reference);
  }
  else // maxCardinality is greater than 1
  {
    ServiceReference<void> serviceToUnbind;
    bool replacementNeeded(false);
    {    
      auto boundRefsHandle = mgr.boundRefs.lock(); // acquire lock on boundRefs
      if (boundRefsHandle->find(reference) == boundRefsHandle->end()) {
        if (!boundRefsHandle->empty()) {
          const ServiceReferenceBase& minBound = *(boundRefsHandle->begin());
          if (minBound < reference) {
            replacementNeeded = true;
            serviceToUnbind = minBound;
          }
        } else {
          replacementNeeded = mgr.IsOptional();
        }
      }
    }

    if(replacementNeeded) {
      mgr.UpdateBoundRefs();
      // bind reference
      notifications.push_back({mgr.metadata.name, RefEvent::BIND_REFERENCE, reference });
      // unbind serviceToUnbind
      notifications.push_back({mgr.metadata.name, RefEvent::UNBIND_REFERENCE, serviceToUnbind });
     
    }
  }
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyDynamicReluctant::ServiceRemoved(const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if (1 == mgr.metadata.maxCardinality)
  {
    notifications = RemoveService(reference);
  }
  else
  {
    // do we need to replace a boundRef
    auto boundRefsHandle = mgr.boundRefs.lock();
    bool needsUnbind = (boundRefsHandle->find(reference) != boundRefsHandle->end());
    if (needsUnbind) {
      auto matchedRefsHandle = mgr.matchedRefs.lock();
      std::set<cppmicroservices::ServiceReferenceBase> notBound;
      
      std::set_difference(matchedRefsHandle->begin(), matchedRefsHandle->end(),
                          boundRefsHandle->begin(), boundRefsHandle->end(),
                          std::inserter(notBound, notBound.begin()));
      auto serviceToBind = *notBound.rbegin(); // best match not in boundRefs;
#if TODO
      // What do I do in the case that notBound is empty? That is, there's no more
      // services left leaving the current service in an incomplete state (that is,
      // there's not enough matching services to deal with the cardinality.
#endif
      boundRefsHandle->insert(serviceToBind);
      boundRefsHandle->erase(reference);
      notifications.push_back({mgr.metadata.name, RefEvent::BIND_REFERENCE, serviceToBind });
      notifications.push_back({mgr.metadata.name, RefEvent::UNBIND_REFERENCE, reference });
    }
  }
  mgr.BatchNotifyAllListeners(notifications);
}

}}
