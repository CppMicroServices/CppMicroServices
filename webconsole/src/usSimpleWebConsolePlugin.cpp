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

#include "usSimpleWebConsolePlugin.h"

#include "usWebConsoleConstants.h"

#include <usServiceProperties.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>

US_BEGIN_NAMESPACE

SimpleWebConsolePlugin::SimpleWebConsolePlugin(const std::string& label, const std::string& title,
                                               const std::string& category, const std::vector<std::string>& css)
  : m_Label(label)
  , m_Title(title)
  , m_Category(category)
  , m_Css(css)
  , m_Context(NULL)
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

SimpleWebConsolePlugin* SimpleWebConsolePlugin::Register(ModuleContext* context)
{
  ServiceProperties props;
  props[WebConsoleConstants::PLUGIN_LABEL()] = GetLabel();
  props[WebConsoleConstants::PLUGIN_TITLE()] = GetTitle();
  if (!GetCategory().empty())
  {
    props[WebConsoleConstants::PLUGIN_CATEGORY()] = GetCategory();
  }
  m_Reg = context->RegisterService<HttpServlet>(this, props);
  m_Context = context;
  return this;
}

void SimpleWebConsolePlugin::Unregister()
{
  if (m_Reg)
  {
    m_Reg.Unregister();
  }
  m_Reg = 0;
  m_Context = NULL;
}

std::vector<std::string> SimpleWebConsolePlugin::GetCssReferences() const
{
  return m_Css;
}

ModuleContext* SimpleWebConsolePlugin::GetContext() const
{
  return m_Context;
}

ModuleResource SimpleWebConsolePlugin::GetResource(const std::string& path) const
{
  return (m_Context != NULL && path.size() > m_LabelRes.size() &&
                       path.compare(0, m_LabelRes.size(), m_LabelRes) == 0) ?
        m_Context->GetModule()->GetResource(path.substr(m_LabelResLen)) :
        ModuleResource();
}

US_END_NAMESPACE
