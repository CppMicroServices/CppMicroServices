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

#ifndef USMODULEFINDHOOK_H
#define USMODULEFINDHOOK_H

#include "usServiceInterface.h"
#include "usShrinkableVector.h"

US_BEGIN_NAMESPACE

class Module;
class ModuleContext;

/**
 * @ingroup MicroServices
 *
 * %Module Context Hook Service.
 *
 * <p>
 * Modules registering this service will be called during module find
 * (get modules) operations.
 *
 * @remarks Implementations of this interface are required to be thread-safe.
 */
struct US_Core_EXPORT ModuleFindHook
{

  virtual ~ModuleFindHook();

  /**
   * Find hook method. This method is called for module find operations
   * using ModuleContext::GetBundle(long)
   * and ModuleContext::GetModules() methods. The find method can
   * filter the result of the find operation.
   *
   * \note A find operation using the ModuleContext::GetModule(const std::string&)
   *       method does not cause the find method to be called, neither does any
   *       call to the static methods of the ModuleRegistry class.
   *
   * @param context The module context of the module performing the find
   *        operation.
   * @param modules A list of Modules to be returned as a result of the
   *        find operation. The implementation of this method may remove
   *        modules from the list to prevent the modules from being
   *        returned to the module performing the find operation.
   */
  virtual void Find(const ModuleContext* context, ShrinkableVector<Module*>& modules) = 0;
};

US_END_NAMESPACE

#endif // USMODULEFINDHOOK_H
