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

#include <usCoreConfig.h>

#include <map>
#include <string>
#include <memory>

namespace us {

class Any;

class Framework;

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
     * are not provided by the configuration argument, the created framework instance will use a reasonable
     * default configuration.
     *
     * @return A new, configured Framework instance.
     */
    std::shared_ptr<Framework> NewFramework(const std::map<std::string, Any>& configuration);

    /**
    * Create a new Framework instance.
    *
    * @remarks The created framework instance will use a reasonable default configuration.
    *
    * @return A new, configured Framework instance.
    */
    std::shared_ptr<Framework> NewFramework(void);

private:
    FrameworkFactory(const FrameworkFactory& );
    FrameworkFactory& operator=(const FrameworkFactory& );

};

}

#endif
