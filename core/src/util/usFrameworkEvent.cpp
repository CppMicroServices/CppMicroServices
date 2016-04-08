/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

namespace us {

class FrameworkEventData : public SharedData
{
public:

  FrameworkEventData& operator=(const FrameworkEventData&) = delete;

  FrameworkEventData(FrameworkEvent::Type type, const std::shared_ptr<Bundle>& bundle, const std::string& message, const std::exception_ptr exception)
    : type(type), bundle(bundle), message(message), exception(exception)
  {

  }

  FrameworkEventData(const FrameworkEventData& other)
    : SharedData(other), type(other.type), bundle(other.bundle), message(other.message), exception(other.exception)
  {

  }

  const FrameworkEvent::Type type;
  const std::shared_ptr<Bundle> bundle;
  const std::string message;
  const std::exception_ptr exception;

};

FrameworkEvent::FrameworkEvent()
  : d(nullptr)
{

}

FrameworkEvent::~FrameworkEvent()
{

}

FrameworkEvent::FrameworkEvent(Type type, const std::shared_ptr<Bundle>& bundle, const std::string& message, const std::exception_ptr exception)
  : d(new FrameworkEventData(type, bundle, message, exception))
{

}

FrameworkEvent::FrameworkEvent(const FrameworkEvent& other)
  : d(other.d)
{

}

FrameworkEvent& FrameworkEvent::operator=(const FrameworkEvent& other)
{
  d = other.d;
  return *this;
}

std::shared_ptr<Bundle> FrameworkEvent::GetBundle() const
{
  if (!d) return nullptr;
  return d->bundle;
}

FrameworkEvent::Type FrameworkEvent::GetType() const
{
  if (!d) return Type::STARTING;
  return d->type;
}

std::string FrameworkEvent::GetMessage() const
{
  if (!d) return std::string();
  return d->message;
}

std::exception_ptr FrameworkEvent::GetThrowable() const
{
  if (!d) return nullptr;
  return d->exception;
}

bool FrameworkEvent::operator!() const
{
  return !d;
}

std::ostream& operator<<(std::ostream& os, FrameworkEvent::Type eventType)
{
  switch (eventType)
  {
  case FrameworkEvent::FRAMEWORK_STARTED:        return os << "STARTED";
  case FrameworkEvent::FRAMEWORK_ERROR:          return os << "ERROR";
  case FrameworkEvent::FRAMEWORK_WARNING:        return os << "WARNING";
  case FrameworkEvent::FRAMEWORK_INFO:           return os << "INFO";
  case FrameworkEvent::FRAMEWORK_STOPPED:        return os << "STOPPED";
  case FrameworkEvent::FRAMEWORK_STOPPED_UPDATE: return os << "STOPPED_UPDATE";
  case FrameworkEvent::FRAMEWORK_WAIT_TIMEDOUT:  return os << "WAIT_TIMEDOUT";

  default: return os << "Unknown bundle event type (" << static_cast<unsigned int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const std::exception_ptr ex)
{
  if (!ex) return os << "NONE";

  try
  {
    std::rethrow_exception(ex);
  }
  catch(const std::exception& e)
  {
    os << e.what();
  }
  catch (...)
  {
    os << "unknown exception";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const FrameworkEvent& evt)
{
  if (!evt) return os << "NONE";

  os << evt.GetType() << "\n " 
      << evt.GetMessage() << "\n " 
      << evt.GetBundle() << "\n Exception: " 
      << evt.GetThrowable();
  return os;
}

}
