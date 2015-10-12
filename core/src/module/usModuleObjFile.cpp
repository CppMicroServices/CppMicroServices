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

#include "usModuleObjFile_p.h"

#include <cstring>

US_BEGIN_NAMESPACE

InvalidObjFileException::InvalidObjFileException(const std::string& what, int errorNumber)
  : m_What(what)
{
  if (errorNumber)
  {
    m_What += std::string(": ") + strerror(errorNumber);
  }
}

const char* InvalidObjFileException::what() const throw()
{
  return m_What.c_str();
}

bool ModuleObjFile::ExtractModuleName(const std::string& name, std::string& out)
{
  static const std::string moduleSignature = "_us_import_module_initializer_";
  if (name.size() > moduleSignature.size() &&
      name.compare(0, moduleSignature.size(), moduleSignature) == 0)
  {
    out = name.substr(moduleSignature.size());
    return true;
  }
  return false;
}

US_END_NAMESPACE
