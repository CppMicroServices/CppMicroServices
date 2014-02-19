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

#include "usServiceProperties.h"

#include <algorithm>

US_BEGIN_NAMESPACE

const std::string& ServiceConstants::OBJECTCLASS()
{
  static const std::string s("objectclass");
  return s;
}

const std::string& ServiceConstants::SERVICE_ID()
{
  static const std::string s("service.id");
  return s;
}

const std::string& ServiceConstants::SERVICE_RANKING()
{
  static const std::string s("service.ranking");
  return s;
}

const std::string& ServiceConstants::SERVICE_SCOPE()
{
  static const std::string s("service.scope");
  return s;
}

const std::string& ServiceConstants::SCOPE_SINGLETON()
{
  static const std::string s("singleton");
  return s;
}

const std::string& ServiceConstants::SCOPE_MODULE()
{
  static const std::string s("module");
  return s;
}

const std::string& ServiceConstants::SCOPE_PROTOTYPE()
{
  static const std::string s("prototype");
  return s;
}

US_END_NAMESPACE

US_USE_NAMESPACE

// make sure all static locals get constructed, so that they
// can be used in destructors of global statics.
std::string tmp1 = ServiceConstants::OBJECTCLASS();
std::string tmp2 = ServiceConstants::SERVICE_ID();
std::string tmp3 = ServiceConstants::SERVICE_RANKING();
std::string tmp4 = ServiceConstants::SERVICE_SCOPE();
std::string tmp5 = ServiceConstants::SCOPE_SINGLETON();
std::string tmp6 = ServiceConstants::SCOPE_MODULE();
std::string tmp7 = ServiceConstants::SCOPE_PROTOTYPE();
