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


#ifndef USCOREMODULECONTEXT_H
#define USCOREMODULECONTEXT_H

#include "usServiceListeners_p.h"
#include "usServiceRegistry_p.h"

US_BEGIN_NAMESPACE

/**
 * This class is not part of the public API.
 */
class CoreModuleContext
{
public:

  /**
   * All listeners in this framework.
   */
  ServiceListeners listeners;

  /**
   * All registered services in this framework.
   */
  ServiceRegistry services;

  /**
   * Contruct a core context
   *
   */
  CoreModuleContext();

  ~CoreModuleContext();

};

US_END_NAMESPACE

#endif // USCOREMODULECONTEXT_H
