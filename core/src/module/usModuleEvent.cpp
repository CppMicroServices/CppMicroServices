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

#include "usModuleEvent.h"

#include "usModule.h"

US_BEGIN_NAMESPACE

class ModuleEventData : public SharedData
{
public:

  ModuleEventData(ModuleEvent::Type type, Module* module)
    : type(type), module(module)
  {

  }

  ModuleEventData(const ModuleEventData& other)
    : SharedData(other), type(other.type), module(other.module)
  {

  }

  const ModuleEvent::Type type;
  Module* const module;

private:

  // purposely not implemented
  ModuleEventData& operator=(const ModuleEventData&);
};

ModuleEvent::ModuleEvent()
  : d(0)
{

}

ModuleEvent::~ModuleEvent()
{

}

bool ModuleEvent::IsNull() const
{
  return !d;
}

ModuleEvent::ModuleEvent(Type type, Module* module)
  : d(new ModuleEventData(type, module))
{

}

ModuleEvent::ModuleEvent(const ModuleEvent& other)
  : d(other.d)
{

}

ModuleEvent& ModuleEvent::operator=(const ModuleEvent& other)
{
  d = other.d;
  return *this;
}

Module* ModuleEvent::GetModule() const
{
  return d->module;
}

ModuleEvent::Type ModuleEvent::GetType() const
{
  return d->type;
}

std::ostream& operator<<(std::ostream& os, ModuleEvent::Type eventType)
{
  switch (eventType)
  {
  case ModuleEvent::LOADED:    return os << "LOADED";
  case ModuleEvent::UNLOADED:  return os << "UNLOADED";
  case ModuleEvent::LOADING:   return os << "LOADING";
  case ModuleEvent::UNLOADING: return os << "UNLOADING";

  default: return os << "Unknown module event type (" << static_cast<int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const ModuleEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  Module* m = event.GetModule();
  os << event.GetType() << " #" << m->GetModuleId() << " (" << m->GetLocation() << ")";
  return os;
}

US_END_NAMESPACE
