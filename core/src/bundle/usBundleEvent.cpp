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
#include "usBundlePrivate.h"

#include <stdexcept>

namespace us {

class BundleEventData : public SharedData
{
public:

  BundleEventData& operator=(const BundleEventData&) = delete;

  BundleEventData(BundleEvent::Type type, const Bundle& bundle,
                   const Bundle& origin)
    : type(type), bundle(GetPrivate(bundle)), origin(GetPrivate(origin))
  {
    if (!bundle) throw std::invalid_argument("invalid bundle");
    if (!origin) throw std::invalid_argument("invalid origin");
  }

  BundleEventData(const BundleEventData& other)
    : SharedData(other), type(other.type), bundle(other.bundle), origin(other.origin)
  {

  }

  const BundleEvent::Type type;

  // Bundle that had a change occur in its lifecycle.
  std::shared_ptr<BundlePrivate> bundle;

  // Bundle that was the origin of the event. For install event type, this is
  // the bundle whose context was used to install the bundle. Otherwise it is
  // the bundle itself.
  std::shared_ptr<BundlePrivate> origin;
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

BundleEvent::BundleEvent(Type type, const Bundle& bundle)
  : d(new BundleEventData(type, bundle, bundle))
{

}

BundleEvent::BundleEvent(Type type, const Bundle& bundle, const Bundle& origin)
  : d(new BundleEventData(type, bundle, origin))
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

Bundle BundleEvent::GetBundle() const
{
  return MakeBundle(d->bundle);
}

BundleEvent::Type BundleEvent::GetType() const
{
  return d->type;
}

std::ostream& operator<<(std::ostream& os, BundleEvent::Type eventType)
{
  switch (eventType)
  {
  case BundleEvent::STARTED:         return os << "STARTED";
  case BundleEvent::STOPPED:         return os << "STOPPED";
  case BundleEvent::STARTING:        return os << "STARTING";
  case BundleEvent::STOPPING:        return os << "STOPPING";
  case BundleEvent::INSTALLED:       return os << "INSTALLED";
  case BundleEvent::UNINSTALLED:     return os << "UNINSTALLED";
  case BundleEvent::RESOLVED:        return os << "RESOLVED";
  case BundleEvent::UNRESOLVED:      return os << "UNRESOLVED";
  case BundleEvent::LAZY_ACTIVATION: return os << "LAZY_ACTIVATION";

  default: return os << "Unknown bundle event type (" << static_cast<int>(eventType) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const BundleEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  auto m = event.GetBundle();
  os << event.GetType() << " #" << m.GetBundleId() << " (" << m.GetLocation() << ")";
  return os;
}

}
