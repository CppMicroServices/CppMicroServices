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

#include "cppmicroservices/webconsole/SimpleWebConsolePlugin.h"

#include "cppmicroservices/webconsole/WebConsoleConstants.h"

#include "cppmicroservices/ServiceProperties.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleResource.h"

namespace cppmicroservices {

SimpleWebConsolePlugin::SimpleWebConsolePlugin(const std::string& label, const std::string& title,
                                               const std::string& category, const std::vector<std::string>& css)
  : m_Label(label)
  , m_Title(title)
  , m_Category(category)
  , m_Css(css)
  , m_Context()
{
  if (label.empty())
  {
    throw std::invalid_argument("Empty label");
  }
  if (title.empty())
  {
    throw std::invalid_argument("Empty title");
  }

  m_LabelRes = "/" + label + "/";
  m_LabelResLen = m_LabelRes.size() - 1;
}

std::string SimpleWebConsolePlugin::GetLabel() const
{
  return m_Label;
}

std::string SimpleWebConsolePlugin::GetTitle() const
{
  return m_Title;
}

std::string SimpleWebConsolePlugin::GetCategory() const
{
  return m_Category;
}

SimpleWebConsolePlugin* SimpleWebConsolePlugin::Register(const BundleContext& context)
{
  ServiceProperties props;
  props[WebConsoleConstants::PLUGIN_LABEL] = GetLabel();
  props[WebConsoleConstants::PLUGIN_TITLE] = GetTitle();
  if (!GetCategory().empty())
  {
    props[WebConsoleConstants::PLUGIN_CATEGORY] = GetCategory();
  }
  m_Context = context;
  m_Reg = m_Context.RegisterService<HttpServlet>(shared_from_this(), props);
  return this;
}

void SimpleWebConsolePlugin::Unregister()
{
  if (m_Reg)
  {
    m_Reg.Unregister();
  }
  m_Reg = 0;
  m_Context = nullptr;
}

std::vector<std::string> SimpleWebConsolePlugin::GetCssReferences() const
{
  return m_Css;
}

BundleContext SimpleWebConsolePlugin::GetContext() const
{
  return m_Context;
}

BundleResource SimpleWebConsolePlugin::GetResource(const std::string& path) const
{
  return (m_Context && path.size() > m_LabelRes.size() &&
          path.compare(0, m_LabelRes.size(), m_LabelRes) == 0) ?
        m_Context.GetBundle().GetResource(path.substr(m_LabelResLen)) :
        BundleResource();
}

}
