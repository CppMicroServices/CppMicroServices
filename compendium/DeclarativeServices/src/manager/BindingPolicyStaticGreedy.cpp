/**
 * @file      BindingPolicyStaticGreedy.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "cppmicroservices/logservice/LogService.hpp"
#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

using namespace cppmicroservices::logservice;

void ReferenceManagerBaseImpl::BindingPolicyStaticGreedy::ServiceAdded(const ServiceReferenceBase& reference)
{
  GreedyServiceAdded(reference);
}

void ReferenceManagerBaseImpl::BindingPolicyStaticGreedy::ServiceRemoved(const ServiceReferenceBase& reference)
{
  mgr.BatchNotifyAllListeners(RemoveService(reference));
}

}}
