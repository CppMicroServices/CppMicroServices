/**
 * @file      BindingPolicy.cpp
 * @copyright Copyright 2020 MathWorks, Inc. 
 */ 

#include "ReferenceManagerImpl.hpp"

namespace cppmicroservices { namespace scrimpl {

bool ReferenceManagerImpl::BindingPolicy::RemoveBoundRef(const ReferenceManagerImpl& mgr
                                                         , const ServiceReferenceBase& reference)
{
  auto boundRefsHandle = mgr.boundRefs.lock();
  auto itr = boundRefsHandle->find(reference);
  return (itr != boundRefsHandle->end());
}

}}
