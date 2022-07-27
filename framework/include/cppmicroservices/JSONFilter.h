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

#ifndef CPPMICROSERVICES_JSONFILTER_H
#define CPPMICROSERVICES_JSONFILTER_H

#include "cppmicroservices/AnyMap.h"
#include <unordered_set> //Jfor JSONExpr

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4251)
#endif

namespace cppmicroservices {

class ServiceReferenceBase;
class Bundle;

/**
\defgroup gr_ldap JSON Filter

\brief Groups JSONFilter class related symbols.
*/

/**
 * \ingroup MicroServices
 * \ingroup gr_ldap
 *
 * An <a href="http://www.ietf.org/rfc/rfc1960.txt">RFC 1960</a>-based Filter.
 *
 * <p>
 * A <code>JSONFilter</code> can be used numerous times to determine if the match
 * argument matches the filter string that was used to create the <code>JSONFilter</code>.
 * <p>
 * Some examples of JSON filters are:
 *
 *   - "(cn=Babs Jensen)"
 *   - "(!(cn=Tim Howes))"
 *   - "(&(" + Constants::OBJECTCLASS + "=Person)(|(sn=Jensen)(cn=Babs J*)))"
 *   - "(o=univ*of*mich*)"
 *
 * \remarks This class is thread safe.
 *
 * \sa JSONProp for a fluent API generating JSON filter strings
 */
class US_Framework_EXPORT JSONFilter
{
// fo JSONExpr
  using ObjectClassSet = std::unordered_set<std::string>;

public:
  /**
   * Creates a valid <code>JSONFilter</code> object that
   * matches nothing.
   *
   */
  JSONFilter();

  /**
   * Creates a <code>JSONFilter</code> object encapsulating the filter string.
   * This <code>JSONFilter</code> object may be used to match a
   * <code>ServiceReference</code> object or a <code>ServiceProperties</code> object.
   *
   * <p>
   * If the filter cannot be parsed, an std::invalid_argument will be
   * thrown with a human readable message where the filter became unparsable.
   *
   * @param filter The filter string.
   * @throws std::invalid_argument If <code>filter</code> contains an invalid
   *         filter string that cannot be parsed.
   * @see "Framework specification for a description of the filter string syntax." TODO!
   */
  JSONFilter(const std::string& filter);

  JSONFilter(const JSONFilter& other);

  ~JSONFilter();

  explicit operator bool() const;

  /**
   * Filter using a service's properties.
   * <p>
   * This <code>JSONFilter</code> is executed using the keys and values of the
   * referenced service's properties. The keys are looked up in a case
   * insensitive manner.
   *
   * @param reference The reference to the service whose properties are used
   *        in the match.
   * @return <code>true</code> if the service's properties match this
   *         <code>JSONFilter</code> <code>false</code> otherwise.
   */
  bool Match(const ServiceReferenceBase& reference) const;

  /**
   * Filter using a bundle's manifest headers.
   * <p>
   * This <code>JSONFilter</code> is executed using the keys and values of the
   * bundle's manifest headers. The keys are looked up in a case insensitive
   * manner.
   *
   * @param bundle The bundle whose manifest's headers are used
   *        in the match.
   * @return <code>true</code> if the bundle's manifest headers match this
   *         <code>JSONFilter</code> <code>false</code> otherwise.
   * @throws std::runtime_error If the number of keys of the bundle's manifest
   *         headers exceeds the value returned by std::numeric_limits<int>::max().
   */
  bool Match(const Bundle& bundle) const;

  
  /** Filter using a <code>AnyMap</code> object with case insensitive key lookup. This
   * <code>JSONFilter</code> is executed using the specified <code>AnyMap</code>'s keys
   * and values. The keys are looked up in a case insensitive manner.
   *
   * @param dictionary The <code>AnyMap</code> whose key/value pairs are used
   *        in the match.
   * @return <code>true</code> if the <code>AnyMap</code>'s values match this
   *         filter; <code>false</code> otherwise.
   * @throws std::runtime_error If the number of keys in the <code>dictionary</code>
   *         exceeds the value returned by std::numeric_limits<int>::max().
   * @throws std::runtime_error If the <code>dictionary</code> contains case variants
   *         of the same key name.
   */
  bool Match(const AnyMap& dictionary) const;

  /**
   * Returns this <code>JSONFilter</code>'s filter string.
   * <p>
   * The filter string is normalized by removing whitespace which does not
   * affect the meaning of the filter.
   *
   * @return This <code>JSONFilter</code>'s filter string.
   */
  std::string ToString() const;

  /**
   * Compares this <code>JSONFilter</code> to another <code>JSONFilter</code>.
   *
   * <p>
   * This implementation returns the result of calling
   * <code>this->ToString() == other.ToString()</code>.
   *
   * @param other The object to compare against this <code>JSONFilter</code>.
   * @return Returns the result of calling
   *         <code>this->ToString() == other.ToString()</code>.
   */
  bool operator==(const JSONFilter& other) const;

  JSONFilter& operator=(const JSONFilter& filter);

// for JSONExpr
  bool GetMatchedObjectClasses(ObjectClassSet& objClasses) const;

 protected:
  std::string filter_str;
};

/**
 * \ingroup MicroServices
 * \ingroup gr_ldap
 *
 * Streams the string representation of \c filter into the stream \c os
 * via JSONFilter::ToString().
 */
US_Framework_EXPORT std::ostream& operator<<(std::ostream& os,
                                             const JSONFilter& filter);
}

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#endif // CPPMICROSERVICES_JSONFILTER_H
