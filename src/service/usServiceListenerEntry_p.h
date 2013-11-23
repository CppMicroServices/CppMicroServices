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


#ifndef USSERVICELISTENERENTRY_H
#define USSERVICELISTENERENTRY_H

#include <usUtils_p.h>
#include <usServiceListenerHook.h>
#include <usListenerFunctors_p.h>

#include "usLDAPExpr_p.h"

US_BEGIN_NAMESPACE

class Module;
class ServiceListenerEntryData;

/**
 * Data structure for saving service listener info. Contains
 * the optional service listener filter, in addition to the info
 * in ListenerEntry.
 */
class ServiceListenerEntry : public ServiceListenerHook::ListenerInfo
{

public:

  typedef US_SERVICE_LISTENER_FUNCTOR ServiceListener;

  ServiceListenerEntry(const ServiceListenerEntry& other);
  ServiceListenerEntry(const ServiceListenerHook::ListenerInfo& info);

  ~ServiceListenerEntry();
  ServiceListenerEntry& operator=(const ServiceListenerEntry& other);

  void SetRemoved(bool removed) const;

  ServiceListenerEntry(ModuleContext* mc, const ServiceListener& l, void* data, const std::string& filter = "");

  const LDAPExpr& GetLDAPExpr() const;

  LDAPExpr::LocalCache& GetLocalCache() const;

  void CallDelegate(const ServiceEvent& event) const;

  bool operator==(const ServiceListenerEntry& other) const;

  std::size_t Hash() const;

};

US_END_NAMESPACE

US_HASH_FUNCTION_NAMESPACE_BEGIN
US_HASH_FUNCTION_BEGIN(US_PREPEND_NAMESPACE(ServiceListenerEntry))
  return arg.Hash();
US_HASH_FUNCTION_END
US_HASH_FUNCTION_NAMESPACE_END

#endif // USSERVICELISTENERENTRY_H
