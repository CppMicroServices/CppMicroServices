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

#include "cppmicroservices/httpservice/ServletConfig.h"
#include "ServletConfigPrivate.h"

namespace cppmicroservices {

ServletConfig::~ServletConfig()
{
}

std::string ServletConfig::GetServletName() const
{
  return d->m_Name;
}

std::shared_ptr<ServletContext> ServletConfig::GetServletContext() const
{
  return d->m_Context;
}

void ServletConfig::SetServletName(const std::string& name)
{
  d->m_Name = name;
}

void ServletConfig::SetServletContext(const std::shared_ptr<ServletContext>& context)
{
  d->m_Context = context;
}

ServletConfig::ServletConfig()
  : d(new ServletConfigPrivate)
{
}

ServletConfig::ServletConfig(const ServletConfig& other)
  : d(other.d)
{
}

ServletConfig&ServletConfig::operator=(const ServletConfig& other)
{
  this->d = other.d;
  return *this;
}

}
