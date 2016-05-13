/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#include "usFrameworkEvent.h"

#include "usBundle.h"

#include <iostream>
#include <stdexcept>

namespace us {

class FrameworkEventData : public SharedData
{
public:

  FrameworkEventData& operator=(const FrameworkEventData&) = delete;

  FrameworkEventData(FrameworkEvent::Type type, const Bundle& bundle,
                     const std::exception_ptr& e)
    : type(type), bundle(bundle), exc(e)
  {
    if (!bundle) throw std::invalid_argument("invalid bundle");
  }

  FrameworkEventData(const FrameworkEventData& other)
    : SharedData(other), type(other.type), bundle(other.bundle), exc(other.exc)
  {
  }

  // Type of event
  const FrameworkEvent::Type type;

  // Bundle related to the event.
  Bundle bundle;

  // Exception related to the event
  std::exception_ptr exc;

};

FrameworkEvent::FrameworkEvent()
  : d(nullptr)
{

}

FrameworkEvent::~FrameworkEvent()
{

}

bool FrameworkEvent::IsNull() const
{
  return !d;
}


FrameworkEvent::FrameworkEvent(const FrameworkEvent& other)
  : d(other.d)
{

}

FrameworkEvent::FrameworkEvent(Type type, const Bundle& bundle, const std::exception_ptr& e)
  : d(new FrameworkEventData(type, bundle, e))
{

}


FrameworkEvent& FrameworkEvent::operator=(const FrameworkEvent& other)
{
  d = other.d;
  return *this;
}

Bundle FrameworkEvent::GetBundle() const
{
  return d->bundle;
}

FrameworkEvent::Type FrameworkEvent::GetType() const
{
  return d->type;
}

std::exception_ptr FrameworkEvent::GetException() const
{
  return d->exc;
}

std::ostream& operator<<(std::ostream& os, FrameworkEvent::Type eventType)
{
  switch (eventType)
  {
  case FrameworkEvent::STARTED:        return os << "STARTED";
  case FrameworkEvent::ERROR:          return os << "ERROR";
  case FrameworkEvent::WARNING:        return os << "WARNING";
  case FrameworkEvent::INFO:           return os << "INFO";
  case FrameworkEvent::STOPPED:        return os << "STOPPED";
  case FrameworkEvent::STOPPED_UPDATE: return os << "STOPPED_UPDATE";
  case FrameworkEvent::WAIT_TIMEDOUT:  return os << "WAIT_TIMEDOUT";

  default: return os << "Unknown framework event type (" << static_cast<int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const FrameworkEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  auto m = event.GetBundle();
  os << event.GetType() << " #" << m.GetBundleId() << " (" << m.GetLocation() << ")";
  return os;
}

}
