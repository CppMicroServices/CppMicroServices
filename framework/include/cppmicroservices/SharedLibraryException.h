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

#ifndef CPPMICROSERVICES_SHAREDLIBRARYEXCEPTION_H
#define CPPMICROSERVICES_SHAREDLIBRARYEXCEPTION_H

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/FrameworkConfig.h"

#include <stdexcept>

namespace cppmicroservices {

class US_Framework_EXPORT SharedLibraryException : public std::runtime_error
{
public:
  explicit SharedLibraryException(const std::string& msg);
                                // TODO: work this in during implementation
                                /*const std::error_code ec,
                                  const std::string& what,
                                  const cppmicroservices::Bundle& origin);*/
  ~SharedLibraryException() override;
  Bundle GetBundle() const;
    
private:
  Bundle origin;  ///< The bundle of the shared library which failed to load.
};

}


#endif /* CPPMICROSERVICES_SHAREDLIBRARYEXCEPTION_H */
