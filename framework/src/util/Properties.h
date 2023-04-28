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

#ifndef CPPMICROSERVICES_PROPERTIES_H
#define CPPMICROSERVICES_PROPERTIES_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/detail/Threads.h"

#include <string>
#include <vector>
#include <unordered_set>

namespace cppmicroservices
{

    class Properties : public detail::MultiThreaded<>
    {

      public:
        explicit Properties(AnyMap const& props);
        explicit Properties(AnyMap&& props);

        Properties(Properties&& o) noexcept;
        Properties& operator=(Properties&& o) noexcept;

        Any const& ValueByRef_unlocked(std::string const& key, bool matchCase = false) const;

        std::pair<Any, bool> Value_unlocked(std::string const& key, bool matchCase = false) const;

        std::vector<std::string> Keys_unlocked() const;

        void Clear_unlocked();

        AnyMap const&
        GetPropsAnyMap() const
        {
            return props;
        }

      private:
        // An AnyMap is used to store the properties rather than 2 vectors (one for keys
        // and the other for values) as previously done in the past. This reduces the number of
        // copies and allows for finds to leverage a map find vs vector find.
        AnyMap props;

        // A case-insensitive map which maps all-lowercased keys to the original key values. This
        // allows for efficient case-insensitive lookups in map types that are not inherently
        // case insensitive.
        mutable std::unordered_set<std::string, detail::any_map_cihash, detail::any_map_ciequal> caseInsensitiveLookup;

        static const Any emptyAny;

        // Helper that populates the case-insensitive lookup map when the provided AnyMap is not
        // already case insensitive.
        void PopulateCaseInsensitiveLookupMap() const;
    };

    class PropertiesHandle
    {
      public:
        PropertiesHandle(Properties const& props, bool lock)
            : props(props)
            , l(lock ? props.Lock() : Properties::UniqueLock())
        {
        }

        PropertiesHandle(PropertiesHandle&& o) noexcept : props(o.props), l(std::move(o.l)) {}

        Properties const*
        operator->() const
        {
            return &props;
        }

      private:
        Properties const& props;
        Properties::UniqueLock l;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_PROPERTIES_H
