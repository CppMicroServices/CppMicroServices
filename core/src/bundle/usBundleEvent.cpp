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

#include "usBundleEvent.h"

#include "usBundle.h"

namespace us {

class BundleEventData : public SharedData
{
public:

  BundleEventData& operator=(const BundleEventData&) = delete;

  BundleEventData(BundleEvent::Type type, const std::shared_ptr<Bundle>& bundle)
    : type(type), bundle(bundle)
  {

  }

  BundleEventData(const BundleEventData& other)
    : SharedData(other), type(other.type), bundle(other.bundle)
  {

  }

  const BundleEvent::Type type;
  const std::shared_ptr<Bundle> bundle;

};

BundleEvent::BundleEvent()
  : d(nullptr)
{

}

BundleEvent::~BundleEvent()
{

}

bool BundleEvent::IsNull() const
{
  return !d;
}

BundleEvent::BundleEvent(Type type, const std::shared_ptr<Bundle>& bundle)
  : d(new BundleEventData(type, bundle))
{

}

BundleEvent::BundleEvent(const BundleEvent& other)
  : d(other.d)
{

}

BundleEvent& BundleEvent::operator=(const BundleEvent& other)
{
  d = other.d;
  return *this;
}

std::shared_ptr<Bundle> BundleEvent::GetBundle() const
{
  return d->bundle;
}

BundleEvent::Type BundleEvent::GetType() const
{
  return d->type;
}

std::ostream& operator<<(std::ostream& os, BundleEvent::Type eventType)
{
  switch (eventType)
  {
  case BundleEvent::STARTED:        return os << "STARTED";
  case BundleEvent::STOPPED:        return os << "STOPPED";
  case BundleEvent::STARTING:       return os << "STARTING";
  case BundleEvent::STOPPING:       return os << "STOPPING";
  case BundleEvent::INSTALLED:      return os << "INSTALLED";
  case BundleEvent::UNINSTALLED:    return os << "UNINSTALLED";

  default: return os << "Unknown bundle event type (" << static_cast<int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const BundleEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  auto m = event.GetBundle();
  os << event.GetType() << " #" << m->GetBundleId() << " (" << m->GetLocation() << "/" << m->GetName() << ")";

  return os;
}

}
