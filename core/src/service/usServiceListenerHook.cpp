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

#include "usServiceListenerHook.h"
#include "usServiceListenerHook_p.h"

US_BEGIN_NAMESPACE

ServiceListenerHook::~ServiceListenerHook()
{
}

ServiceListenerHook::ListenerInfoData::ListenerInfoData(
    ModuleContext* mc, const ServiceListenerEntry::ServiceListener& l,
    void* data, const std::string& filter)
  : mc(mc)
  , listener(l)
  , data(data)
  , filter(filter)
  , bRemoved(false)
{
}

ServiceListenerHook::ListenerInfoData::~ListenerInfoData()
{
}

ServiceListenerHook::ListenerInfo::ListenerInfo(ListenerInfoData* data)
  : d(data)
{
}

ServiceListenerHook::ListenerInfo::ListenerInfo()
  : d(NULL)
{
}

ServiceListenerHook::ListenerInfo::ListenerInfo(const ListenerInfo& other)
  : d(other.d)
{
}

ServiceListenerHook::ListenerInfo::~ListenerInfo()
{
}

ServiceListenerHook::ListenerInfo& ServiceListenerHook::ListenerInfo::operator=(const ListenerInfo& other)
{
  d = other.d;
  return *this;
}

bool ServiceListenerHook::ListenerInfo::IsNull() const
{
  return !d;
}

ModuleContext* ServiceListenerHook::ListenerInfo::GetModuleContext() const
{
  return d->mc;
}

std::string ServiceListenerHook::ListenerInfo::GetFilter() const
{
  return d->filter;
}

bool ServiceListenerHook::ListenerInfo::IsRemoved() const
{
  return d->bRemoved;
}

bool ServiceListenerHook::ListenerInfo::operator==(const ListenerInfo& other) const
{
  return d == other.d;
}

US_END_NAMESPACE
