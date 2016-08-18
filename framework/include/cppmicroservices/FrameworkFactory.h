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

#ifndef CPPMICROSERVICES_FRAMEWORKFACTORY_H
#define CPPMICROSERVICES_FRAMEWORKFACTORY_H

#include "cppmicroservices/FrameworkConfig.h"

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace cppmicroservices {

class Any;

class Framework;

/**
 * \ingroup MicroServices
 *
 * A factory for creating Framework instances.
 *
 * @remarks This class is thread-safe.
 */
class US_Framework_EXPORT FrameworkFactory
{
public:
    /**
     * Create a new Framework instance.
     *
     * @param configuration The framework properties to configure the new framework instance. If framework properties
     * are not provided by the configuration argument, the created framework instance will use a reasonable
     * default configuration.
     * @param logger Any ostream object which will receieve redirected debug log output.
     *
     * @return A new, configured Framework instance.
     */
    Framework NewFramework(const std::map<std::string, Any>& configuration = std::map<std::string, Any>(), std::ostream* logger = nullptr);

};

}

#endif // CPPMICROSERVICES_FRAMEWORKFACTORY_H
