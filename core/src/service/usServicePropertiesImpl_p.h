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

#ifndef USSERVICEPROPERTIESIMPL_P_H
#define USSERVICEPROPERTIESIMPL_P_H

#include "usServiceProperties.h"

US_BEGIN_NAMESPACE

class ServicePropertiesImpl
{

public:

  explicit ServicePropertiesImpl(const ServiceProperties& props);

  const Any& Value(const std::string& key) const;
  const Any& Value(int index) const;

  int Find(const std::string& key) const;
  int FindCaseSensitive(const std::string& key) const;

  const std::vector<std::string>& Keys() const;

private:

  std::vector<std::string> keys;
  std::vector<Any> values;

  static Any emptyAny;

};

US_END_NAMESPACE

#endif // USSERVICEPROPERTIESIMPL_P_H
