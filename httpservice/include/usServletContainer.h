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

#ifndef USSERVLETCONTAINER_H
#define USSERVLETCONTAINER_H

#include "usHttpServiceExport.h"

#include <string>
#include <memory>

namespace us {

struct ServletContainerPrivate;
class ServletContext;

class US_HttpService_EXPORT ServletContainer
{
public:
  ServletContainer(const std::string& contextPath = std::string());
  ~ServletContainer();

  void SetContextPath(const std::string& contextPath);
  std::string GetContextPath() const;

  void Start();
  void Stop();

  std::shared_ptr<ServletContext> GetContext(const std::string& uripath) const;
  std::string GetContextPath(const ServletContext* context) const;

private:

  friend class ServletContext;

  ServletContainerPrivate* d;
};

}

#endif // USSERVLETCONTAINER_H
