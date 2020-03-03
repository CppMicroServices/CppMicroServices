/**
 * @file      BindingPolicyStaticReluctant.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyStaticReluctant::ServiceAdded(const ServiceReferenceBase& reference)
{
  auto notifications = ReluctantServiceAdded(reference);
  mgr.BatchNotifyAllListeners(notifications);
}

void ReferenceManagerBaseImpl::BindingPolicyStaticReluctant::ServiceRemoved(const ServiceReferenceBase& reference)
{
  auto notifications = RemoveService(reference);
  mgr.BatchNotifyAllListeners(notifications);
}

}} // namespaces
