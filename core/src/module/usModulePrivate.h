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

#ifndef USMODULEPRIVATE_H
#define USMODULEPRIVATE_H

#include <map>
#include <list>

#include "usModuleRegistry.h"
#include "usModuleVersion.h"
#include "usModuleInfo.h"
#include "usModuleManifest_p.h"
#include "usModuleResourceContainer_p.h"
#include "usSharedLibrary.h"

#include "usAtomicInt_p.h"

US_BEGIN_NAMESPACE

class CoreModuleContext;
class ModuleContext;
struct ModuleActivator;

/**
 * \ingroup MicroServices
 */
class ModulePrivate {

public:

  /**
   * Construct a new module based on a ModuleInfo object.
   */
  ModulePrivate(Module* qq, CoreModuleContext* coreCtx, ModuleInfo* info);

  virtual ~ModulePrivate();

  void RemoveModuleResources();

  CoreModuleContext* const coreCtx;

  /**
   * Module version
   */
  ModuleVersion version;

  ModuleInfo info;

  ModuleResourceContainer resourceContainer;

  /**
   * ModuleContext for the module
   */
  ModuleContext* moduleContext;

  ModuleActivator* moduleActivator;

  ModuleManifest moduleManifest;

  std::string baseStoragePath;
  std::string storagePath;

  Module* const q;

  // responsible for platform specific loading and unloading
  // of the bundle's physical form.
  SharedLibrary lib;

private:

  void InitializeResources();

  // purposely not implemented
  ModulePrivate(const ModulePrivate&);
  ModulePrivate& operator=(const ModulePrivate&);

};

US_END_NAMESPACE

#endif // USMODULEPRIVATE_H
