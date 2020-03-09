/**
 * @file      BindingPolicyDynamicGreedy.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

void ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy::ServiceAdded(const ServiceReferenceBase& reference)
{
  GreedyServiceAdded(reference);
}

void ReferenceManagerBaseImpl::BindingPolicyDynamicGreedy::ServiceRemoved(const ServiceReferenceBase& reference)
{
  mgr.BatchNotifyAllListeners(RemoveService(reference));
}

}}
