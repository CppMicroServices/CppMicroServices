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

#ifndef USFRAMEWORKFACTORY_H
#define USFRAMEWORKFACTORY_H

#include "usGlobalConfig.h"
#include "usFramework.h"

US_BEGIN_NAMESPACE

/**
 * \ingroup MicroServices
 *
 * A factory for creating Framework instances.
 *
 * @remarks This class is thread-safe.
 */
class US_Core_EXPORT FrameworkFactory
{
public:
    FrameworkFactory(void);
    virtual ~FrameworkFactory(void);

    /**
     * Create a new Framework instance.
     * 
     * @param configuration The framework properties to configure the new framework instance. If framework properties
     * are not provided by the configuration argument, the created framework instance must use some reasonable
     * default configuration. The created framework instance must copy any information needed from the specified 
     * configuration argument since the configuration argument can be changed after the framework instance has been created.
     *
     * @return A new, configured Framework instance.
     */
    Framework* newFramework(std::map<std::string, std::string> configuration);

private:
    FrameworkFactory(const FrameworkFactory& );
    FrameworkFactory& operator=(const FrameworkFactory& );
};

US_END_NAMESPACE

#endif