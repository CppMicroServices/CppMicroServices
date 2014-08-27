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

#include "usServiceEvent.h"

#include "usServiceProperties.h"

US_BEGIN_NAMESPACE

class ServiceEventData : public SharedData
{
public:

  ServiceEventData(const ServiceEvent::Type& type, const ServiceReferenceBase& reference)
    : type(type), reference(reference)
  {

  }

  ServiceEventData(const ServiceEventData& other)
    : SharedData(other), type(other.type), reference(other.reference)
  {

  }

  const ServiceEvent::Type type;
  const ServiceReferenceBase reference;

private:

  // purposely not implemented
  ServiceEventData& operator=(const ServiceEventData&);
};

ServiceEvent::ServiceEvent()
  : d(0)
{

}

ServiceEvent::~ServiceEvent()
{

}

bool ServiceEvent::IsNull() const
{
  return !d;
}

ServiceEvent::ServiceEvent(Type type, const ServiceReferenceBase& reference)
  : d(new ServiceEventData(type, reference))
{

}

ServiceEvent::ServiceEvent(const ServiceEvent& other)
  : d(other.d)
{

}

ServiceEvent& ServiceEvent::operator=(const ServiceEvent& other)
{
  d = other.d;
  return *this;
}

ServiceReferenceU ServiceEvent::GetServiceReference() const
{
  return d->reference;
}

ServiceEvent::Type ServiceEvent::GetType() const
{
  return d->type;
}

std::ostream& operator<<(std::ostream& os, const ServiceEvent::Type& type)
{
  switch(type)
  {
  case ServiceEvent::MODIFIED:          return os << "MODIFIED";
  case ServiceEvent::MODIFIED_ENDMATCH: return os << "MODIFIED_ENDMATCH";
  case ServiceEvent::REGISTERED:        return os << "REGISTERED";
  case ServiceEvent::UNREGISTERING:     return os << "UNREGISTERING";

  default: return os << "unknown service event type (" << static_cast<int>(type) << ")";
  }
}

std::ostream& operator<<(std::ostream& os, const ServiceEvent& event)
{
  if (event.IsNull()) return os << "NONE";

  os << event.GetType();

  ServiceReferenceU sr = event.GetServiceReference();
  if (sr)
  {
    // Some events will not have a service reference
    long int sid = any_cast<long int>(sr.GetProperty(ServiceConstants::SERVICE_ID()));
    os << " " << sid;

    Any classes = sr.GetProperty(ServiceConstants::OBJECTCLASS());
    os << " objectClass=" << classes.ToString() << ")";
  }

  return os;
}

US_END_NAMESPACE
