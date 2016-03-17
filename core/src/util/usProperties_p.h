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

#ifndef USPROPERTIES_P_H
#define USPROPERTIES_P_H

#include <vector>
#include <map>
#include <string>

#include "usAny.h"
#include "usServiceProperties.h"
#include "usThreads_p.h"

namespace us {

class Properties : public MultiThreaded<>
{

public:

  explicit Properties(const ServiceProperties& props);
  Properties(const std::map<std::string, Any>& p);

  Properties(Properties&& o);
  Properties& operator=(Properties&& o);

  Any Value_unlocked(const std::string& key) const;
  Any Value_unlocked(int index) const;

  int Find_unlocked(const std::string& key) const;
  int FindCaseSensitive_unlocked(const std::string& key) const;

  std::vector<std::string> Keys_unlocked() const;

  void Clear_unlocked();

private:

  std::vector<std::string> keys;
  std::vector<Any> values;

  static const Any emptyAny;

  template <typename T>
  void ConstructProperties(const T& p);

};

class PropertiesHandle
{
public:
  PropertiesHandle(const Properties& props, bool lock)
    : props(props)
    , l(lock ? props.Lock() : Properties::UniqueLock())
  {}

  PropertiesHandle(PropertiesHandle&& o)
    : props(o.props)
    , l(std::move(o.l))
  {}

  const Properties* operator-> () const { return &props; }

private:

  const Properties& props;
  Properties::UniqueLock l;
};

}

#endif // USPROPERTIES_P_H
