/**
 * @file      BindingPolicy.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "ReferenceManagerImpl.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

namespace cppmicroservices { namespace scrimpl {

void ReferenceManagerBaseImpl::BindingPolicy::Log(std::string&& logStr, cppmicroservices::logservice::SeverityLevel logLevel)
{
  mgr.logger->Log(logLevel, logStr);
}

bool ReferenceManagerBaseImpl::BindingPolicy::ShouldClearBoundRefs(const ServiceReferenceBase& reference)
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  auto itr = boundRefsHandle->find(reference);
  bool rval = (itr != boundRefsHandle->end());
  return rval;
}

bool ReferenceManagerBaseImpl::BindingPolicy::ShouldNotifySatisfied()
{
  bool rval = (mgr.IsSatisfied()
               ? false
               : mgr.UpdateBoundRefs());
  return rval;
}

void ReferenceManagerBaseImpl::BindingPolicy::ClearBoundRefs()
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  boundRefsHandle->clear();
}

std::vector<RefChangeNotification> ReferenceManagerBaseImpl::BindingPolicy::ReluctantServiceAdded(const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if(!reference)
  {
    Log("ServiceAdded: service with id "
        + std::to_string(GetServiceId(reference))
        + " has already been unregistered, no-op");
    
  }
  else
  {
    auto notifySatisfied = ShouldNotifySatisfied();
    if(notifySatisfied)
    {
      Log("Notify SATISFIED for reference " + mgr.metadata.name);
      notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
    }
  }
  return notifications;
}

std::vector<RefChangeNotification> ReferenceManagerBaseImpl::BindingPolicy::RemoveService(const ServiceReferenceBase& reference)
{
  std::vector<RefChangeNotification> notifications;
  if(ShouldClearBoundRefs(reference)) {
    Log("Notify UNSATISFIED for reference " + mgr.metadata.name);
    notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_UNSATISFIED, reference });

    ClearBoundRefs();
    if (mgr.UpdateBoundRefs())
    {
      Log("Notify SATISFIED for reference " + mgr.metadata.name);
      notifications.push_back({ mgr.metadata.name, RefEvent::BECAME_SATISFIED, reference });
    }
  }
  return notifications;
}

}}
