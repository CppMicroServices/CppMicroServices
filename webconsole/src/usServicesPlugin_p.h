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

#ifndef USSERVICESPLUGIN_H
#define USSERVICESPLUGIN_H

#include "usSimpleWebConsolePlugin.h"

US_BEGIN_NAMESPACE

class ModuleResourceStream;

class ServicesPlugin : public SimpleWebConsolePlugin
{
public:
  ServicesPlugin();
  ~ServicesPlugin();

private:

  bool IsHtmlRequest(HttpServletRequest &request);

  void RenderContent(HttpServletRequest& /*request*/, HttpServletResponse& response);

  std::string GetIds_JSON() const;
  std::string GetInterface_JSON(const std::string& iid) const;

  ModuleResourceStream* m_TemplateRS;
  ModuleResourceStream* m_TemplateSI;
};

US_END_NAMESPACE

#endif // USSERVICESPLUGIN_H
