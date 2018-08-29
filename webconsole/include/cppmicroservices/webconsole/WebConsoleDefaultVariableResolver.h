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

#ifndef CPPMICROSERVICES_WEBCONSOLEDEFAULTVARIABLERESOLVER_H
#define CPPMICROSERVICES_WEBCONSOLEDEFAULTVARIABLERESOLVER_H

#include "cppmicroservices/webconsole/WebConsoleExport.h"
#include "cppmicroservices/webconsole/WebConsoleVariableResolver.h"
#include "cppmicroservices/webconsole/mustache.hpp"

#include <map>

namespace cppmicroservices {

using MustacheData = Kainjow::Mustache::Data;

/**
 * The default Web Console variable resolver class.
 *
 * This variable resolver uses Mustache template logic to resolve
 * variables.
 */
class US_WebConsole_EXPORT WebConsoleDefaultVariableResolver
  : public WebConsoleVariableResolver
{
public:
  virtual std::string Resolve(const std::string& variable) const;

  MustacheData& GetData();

private:
  MustacheData m_Data;
};
}

#endif // CPPMICROSERVICES_WEBCONSOLEDEFAULTVARIABLERESOLVER_H
