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

#ifndef CPPMICROSERVICES_SERVICESPLUGIN_P_H
#define CPPMICROSERVICES_SERVICESPLUGIN_P_H

#include "cppmicroservices/webconsole/SimpleWebConsolePlugin.h"

namespace cppmicroservices {

class ServicesPlugin : public SimpleWebConsolePlugin
{
public:
  ServicesPlugin();

private:

  bool IsHtmlRequest(HttpServletRequest &request);

  void RenderContent(HttpServletRequest& /*request*/, HttpServletResponse& response);

  std::string GetIds_JSON() const;
  std::string GetInterface_JSON(const std::string& iid) const;

};

}

#endif // CPPMICROSERVICES_SERVICESPLUGIN_P_H
