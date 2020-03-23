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

#ifndef CPPMICROSERVICES_IHTTPSERVLETPART_H
#define CPPMICROSERVICES_IHTTPSERVLETPART_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/SharedData.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <istream>
#include <string>
#include <vector>

namespace cppmicroservices {

/**
 * Store one part of a multipart request. Follows the implementation
 * javax.servlet.http.Part
 * https://docs.oracle.com/javaee/7/api/javax/servlet/http/Part.html
 */
class US_HttpService_EXPORT IHttpServletPart
{
public:
  virtual ~IHttpServletPart() = default;

  /**
   * Deletes the underlying storage for a file item, including deleting any
   * associated temporary disk file.
   */
  virtual void Delete() = 0;

  /**
   * Gets the content type of this part.
   */
  virtual std::string GetContentType() const = 0;

  /**
   * Returns the value of the specified mime header as a std::string.
   */
  virtual std::string GetHeader(const std::string& name) const = 0;

  /**
   * Gets the header names of this Part.
   */
  virtual std::vector<std::string> GetHeaderNames() const = 0;

  /**
   * Gets the values of the Part header with the given name.
   */
  virtual std::vector<std::string> GetHeaders(
    const std::string& name) const = 0;

  /**
   * Gets the content of this part as an InputStream
   */
  virtual std::istream* GetInputStream() = 0;

  /**
   * Gets the name of this part
   */
  virtual std::string GetName() const = 0;

  /**
   * Returns the size of this file.
   */
  virtual long long GetSize() const = 0;

  /**
   * Gets the file name specified by the client
   */
  virtual std::string GetSubmittedFileName() const = 0;

  /**
   * A convenience method to write this uploaded item to disk.
   */
  virtual void Write(const std::string& fileName) = 0;

  static const std::string NO_FILE;
};
}

#endif // CPPMICROSERVICES_IHTTPREQUEST_H
