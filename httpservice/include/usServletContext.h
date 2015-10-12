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

#ifndef USSERVLETCONTEXT_H
#define USSERVLETCONTEXT_H

#include "usHttpServiceExport.h"
#include <usGlobalConfig.h>

#include <string>

namespace us {

class ServletContainer;

class US_HttpService_EXPORT ServletContext
{
public:

  std::string GetContextPath() const;

  ServletContext* GetContext(const std::string& uripath);

  std::string GetMimeType(const std::string& file) const;

private:

  friend struct ServletContainerPrivate;

  ServletContext(ServletContainer* container);
  ServletContainer* m_Container;
};

}

#endif // USSERVLETCONTEXT_H
