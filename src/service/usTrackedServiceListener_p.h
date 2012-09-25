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


#ifndef USTRACKEDSERVICELISTENER_H
#define USTRACKEDSERVICELISTENER_H

#include "usServiceEvent.h"

US_BEGIN_NAMESPACE

/**
 * This class is not intended to be used directly. It is exported to support
 * the CppMicroServices module system.
 */
struct TrackedServiceListener // : public US_BASECLASS_NAME
{
  virtual ~TrackedServiceListener() {}

  /**
   * Slot connected to service events for the
   * <code>ServiceTracker</code> class. This method must NOT be
   * synchronized to avoid deadlock potential.
   *
   * @param event <code>ServiceEvent</code> object from the framework.
   */
  virtual void ServiceChanged(const ServiceEvent event) = 0;

};

US_END_NAMESPACE

#endif // USTRACKEDSERVICELISTENER_H
