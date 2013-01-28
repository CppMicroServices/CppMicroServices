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
#include <usModuleContext.h>

#include "usLDAPExpr_p.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4396)
#endif

US_BEGIN_NAMESPACE

class Module;
class ServiceListenerEntryData;

/**
 * Data structure for saving service listener info. Contains
 * the optional service listener filter, in addition to the info
 * in ListenerEntry.
 */
class ServiceListenerEntry
{

public:

  typedef US_SERVICE_LISTENER_FUNCTOR ServiceListener;

  ServiceListenerEntry(const ServiceListenerEntry& other);
  ~ServiceListenerEntry();
  ServiceListenerEntry& operator=(const ServiceListenerEntry& other);

  bool operator==(const ServiceListenerEntry& other) const;

  bool operator<(const ServiceListenerEntry& other) const;

  void SetRemoved(bool removed) const;
  bool IsRemoved() const;

  ServiceListenerEntry(Module* mc, const ServiceListener& l, void* data, const std::string& filter = "");

  Module* GetModule() const;

  std::string GetFilter() const;

  LDAPExpr GetLDAPExpr() const;

  LDAPExpr::LocalCache& GetLocalCache() const;

  void CallDelegate(const ServiceEvent& event) const;

private:

  US_HASH_FUNCTION_FRIEND(ServiceListenerEntry);

  ExplicitlySharedDataPointer<ServiceListenerEntryData> d;
};

US_END_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif

US_HASH_FUNCTION_NAMESPACE_BEGIN
US_HASH_FUNCTION_BEGIN(US_PREPEND_NAMESPACE(ServiceListenerEntry))

return US_HASH_FUNCTION(const US_PREPEND_NAMESPACE(ServiceListenerEntryData)*, arg.d.ConstData());

US_HASH_FUNCTION_END
US_HASH_FUNCTION_NAMESPACE_END

#endif // USSERVICELISTENERENTRY_H
