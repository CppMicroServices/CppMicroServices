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

#ifndef CPPMICROSERVICES_ANYMAP_H
#define CPPMICROSERVICES_ANYMAP_H

#include "cppmicroservices/Any.h"

#include <unordered_map>
#include <string>

namespace cppmicroservices {

struct ci_cmp {
    bool operator()(const std::string& a, const std::string& b) const
    {
        bool retVal = false;
        if(a.length() == b.length())
        {
            for(size_t i = 0; i < a.length(); i++)
            {
                char aChar = std::toupper(a.at(i));
                char bChar = std::toupper(b.at(i));
                if(aChar != bChar)
                {
                    retVal = (aChar < bChar);
                    break;
                }
            }
        }
        else
        {
            retVal = (a.length() < b.length());
        }
        return retVal;
    }
};

typedef std::map<std::string, Any, ci_cmp> any_map;

/**
 * \ingroup MicroServicesUtils
 *
 * A map data structure with support for compound keys.
 *
 * This class adds convenience functions on top of the \c any_map
 * class. The \c any_map is a recursive data structure, and its values can be
 * retrieved via standard map functions or by using a dotted key notation
 * specifying a compound key.
 *
 * @see any_map
 */
class US_Framework_EXPORT AnyMap : public any_map
{
public:
#if 0
  AnyMap(map_type type);
  AnyMap(const ordered_any_map& m);
  AnyMap(const unordered_any_map& m);
  AnyMap(const unordered_any_cimap& m);

  /**
   * Get the underlying STL container type.
   *
   * @return The STL container type holding the map data.
   */
  map_type GetType() const;
#endif
  /**
   * Get a key's value, using a compound key notation.
   *
   * A compound key consists of one or more key names, concatenated with
   * the '.' (dot) character. Each key except the last requires the referenced
   * Any object to be of type \c AnyMap or \c std::vector<Any>. Containers
   * of type \c std::vector<Any> are indexed using 0-based numerical key names.
   *
   * For example, a \c AnyMap object holding data of the following layout
   * \code{.json}
   * {
   *   one: 1,
   *   two: "two",
   *   three: {
   *     a: "anton",
   *     b: [ 3, 8 ]
   *   }
   * }
   * \endcode
   * can be queried using the following notation:
   * \code
   * map.AtCompoundKey("one");       // returns Any(1)
   * map.AtCompoundKey("three.a");   // returns Any(std::string("anton"))
   * map.AtCompoundKey("three.b.1"); // returns Any(8)
   * \endcode
   *
   * @param key The key hierachy to query.
   * @return A reference to the key's value.
   *
   * @throws std::invalid_argument if the \c Any value for a given key is not of type \c AnyMap or \c std::vector<Any>.
   * std::out_of_range if the key is not found or a numerical index would fall out of the range of an \c int type.
   */
  mapped_type& AtCompoundKey(const key_type& key);
  const mapped_type& AtCompoundKey(const key_type& key) const;

};

template<>
US_Framework_EXPORT std::ostream& any_value_to_string(std::ostream& os, const AnyMap& m);

}

#endif // CPPMICROSERVICES_ANYMAP_H
