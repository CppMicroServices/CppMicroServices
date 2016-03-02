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

#include "usSettingsPlugin_p.h"

#include "usHttpServletRequest.h"
#include "usHttpServletResponse.h"

#include "usGetBundleContext.h"
#include "usBundleContext.h"
#include "usBundle.h"
#include "usBundleResource.h"
#include "usBundleResourceStream.h"

namespace us {

static std::string LABEL()
{
  static std::string s = "settings";
  return s;
}

static std::string TITLE()
{
  static std::string s = "CppMicroServices Settings";
  return s;
}

static std::string CATEGORY()
{
  static std::string s;
  return s;
}

SettingsPlugin::SettingsPlugin()
  : SimpleWebConsolePlugin(LABEL(), TITLE(), CATEGORY())
  , m_TemplateRS(nullptr)
{
}

SettingsPlugin::~SettingsPlugin()
{
  delete m_TemplateRS;
}

void SettingsPlugin::RenderContent(HttpServletRequest&, HttpServletResponse& response)
{
  if (m_TemplateRS == nullptr)
  {
    BundleResource res = GetBundleContext()->GetBundle()->GetResource("/templates/settings.html");
    m_TemplateRS = new BundleResourceStream(res, std::ios_base::binary);
  }
  m_TemplateRS->seekg(0, std::ios_base::beg);
  response.GetOutputStream() << m_TemplateRS->rdbuf();
}

BundleResource SettingsPlugin::GetResource(const std::string& path) const
{
  return (this->GetContext() != nullptr) ?
        this->GetContext()->GetBundle()->GetResource(path) :
        BundleResource();
}

}
