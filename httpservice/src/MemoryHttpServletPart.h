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

#ifndef CPPMICROSERVICES_HTTPSERVLETPART_H
#define CPPMICROSERVICES_HTTPSERVLETPART_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/SharedData.h"
#include "cppmicroservices/httpservice/HttpServiceExport.h"

#include <istream>
#include <string>
#include <vector>

namespace cppmicroservices {

struct HttpServletPartPrivate;
class ServletContext;
class HttpServletRequest;

/**
 * Store one part of a multipart request. Follows the implementation
 * javax.servlet.http.Part
 * https://docs.oracle.com/javaee/7/api/javax/servlet/http/Part.html
 */
class US_HttpService_EXPORT HttpServletPart
{
public:
  ~HttpServletPart();

  HttpServletPart(const HttpServletPart& o);
  HttpServletPart& operator=(const HttpServletPart& o);

  /**
   * Deletes the underlying storage for a file item, including deleting any
   * associated temporary disk file.
   */
  void Delete();

  /**
   * Gets the content type of this part.
   */
  std::string GetContentType() const;

  /**
   * Returns the value of the specified mime header as a std::string.
   */
  std::string GetHeader(const std::string& name) const;

  /**
   * Gets the header names of this Part.
   */
  std::vector<std::string> GetHeaderNames() const;

  /**
   * Gets the values of the Part header with the given name.
   */
  std::vector<std::string> GetHeaders(const std::string& name) const;

  /**
   * Gets the content of this part as an InputStream
   */
  std::istream* GetInputStream();

  /**
   * Gets the name of this part
   */
  std::string GetName() const;

  /**
   * Returns the size of this file.
   */
  long long GetSize() const;

  /**
   * Gets the file name specified by the client
   */
  std::string GetSubmittedFileName() const;

  /**
   * A convenience method to write this uploaded item to disk.
   */
  void Write(const std::string& fileName);

private:
  friend class ServletHandler;
  friend class HttpServletRequest;
  HttpServletPart(HttpServletPartPrivate* d);

  ExplicitlySharedDataPointer<HttpServletPartPrivate> d;
};
}

#endif // CPPMICROSERVICES_HTTPREQUEST_H
