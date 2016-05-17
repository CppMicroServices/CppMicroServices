/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#ifndef USBUNDLEMANIFEST_P_H
#define USBUNDLEMANIFEST_P_H

#include "usAny.h"

namespace us {

class BundleManifest
{
public:

  BundleManifest();

  void Parse(std::istream& is);

  bool Contains(const std::string& key) const;

  Any GetValue(const std::string& key) const;

  std::vector<std::string> GetKeys() const;

  void SetValue(const std::string& key, const Any& value);

private:

  typedef std::map<std::string, Any> AnyMap;

  AnyMap m_Properties;
};

}

#endif // USBUNDLEMANIFEST_P_H
