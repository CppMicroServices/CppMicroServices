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

#ifndef USLDAPFILTER_H
#define USLDAPFILTER_H

#include "usServiceReference.h"
#include "usServiceProperties.h"

#include "usSharedData.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

US_BEGIN_NAMESPACE

class LDAPFilterData;

/**
 * \ingroup MicroServices
 *
 * An <a href="http://www.ietf.org/rfc/rfc1960.txt">RFC 1960</a>-based Filter.
 *
 * <p>
 * A <code>LDAPFilter</code> can be used numerous times to determine if the match
 * argument matches the filter string that was used to create the <code>LDAPFilter</code>.
 * <p>
 * Some examples of LDAP filters are:
 *
 *   - "(cn=Babs Jensen)"
 *   - "(!(cn=Tim Howes))"
 *   - "(&(" + ServiceConstants::OBJECTCLASS() + "=Person)(|(sn=Jensen)(cn=Babs J*)))"
 *   - "(o=univ*of*mich*)"
 *
 * \remarks This class is thread safe.
 */
class US_EXPORT LDAPFilter {

public:

  /**
   * Creates in invalid <code>LDAPFilter</code> object.
   * Test the validity by using the boolean conversion operator.
   *
   * <p>
   * Calling methods on an invalid <code>LDAPFilter</code>
   * will result in undefined behavior.
   */
  LDAPFilter();

  /**
   * Creates a <code>LDAPFilter</code> object. This <code>LDAPFilter</code>
   * object may be used to match a <code>ServiceReference</code> object or a
   * <code>ServiceProperties</code> object.
   *
   * <p>
   * If the filter cannot be parsed, an std::invalid_argument will be
   * thrown with a human readable message where the filter became unparsable.
   *
   * @param filter The filter string.
   * @return A <code>LDAPFilter</code> object encapsulating the filter string.
   * @throws std::invalid_argument If <code>filter</code> contains an invalid
   *         filter string that cannot be parsed.
   * @see "Framework specification for a description of the filter string syntax." TODO!
   */
  LDAPFilter(const std::string& filter);

  LDAPFilter(const LDAPFilter& other);

  ~LDAPFilter();

  operator bool() const;

  /**
   * Filter using a service's properties.
   * <p>
   * This <code>LDAPFilter</code> is executed using the keys and values of the
   * referenced service's properties. The keys are looked up in a case
   * insensitive manner.
   *
   * @param reference The reference to the service whose properties are used
   *        in the match.
   * @return <code>true</code> if the service's properties match this
   *         <code>LDAPFilter</code> <code>false</code> otherwise.
   */
  bool Match(const ServiceReference& reference) const;

  /**
   * Filter using a <code>ServiceProperties</code> object with case insensitive key lookup. This
   * <code>LDAPFilter</code> is executed using the specified <code>ServiceProperties</code>'s keys
   * and values. The keys are looked up in a case insensitive manner.
   *
   * @param dictionary The <code>ServiceProperties</code> whose key/value pairs are used
   *        in the match.
   * @return <code>true</code> if the <code>ServiceProperties</code>'s values match this
   *         filter; <code>false</code> otherwise.
   */
  bool Match(const ServiceProperties& dictionary) const;

  /**
   * Filter using a <code>ServiceProperties</code>. This <code>LDAPFilter</code> is executed using
   * the specified <code>ServiceProperties</code>'s keys and values. The keys are looked
   * up in a normal manner respecting case.
   *
   * @param dictionary The <code>ServiceProperties</code> whose key/value pairs are used
   *        in the match.
   * @return <code>true</code> if the <code>ServiceProperties</code>'s values match this
   *         filter; <code>false</code> otherwise.
   */
  bool MatchCase(const ServiceProperties& dictionary) const;

  /**
   * Returns this <code>LDAPFilter</code>'s filter string.
   * <p>
   * The filter string is normalized by removing whitespace which does not
   * affect the meaning of the filter.
   *
   * @return This <code>LDAPFilter</code>'s filter string.
   */
  std::string ToString() const;

  /**
   * Compares this <code>LDAPFilter</code> to another <code>LDAPFilter</code>.
   *
   * <p>
   * This implementation returns the result of calling
   * <code>this->ToString() == other.ToString()</code>.
   *
   * @param other The object to compare against this <code>LDAPFilter</code>.
   * @return Returns the result of calling
   *         <code>this->ToString() == other.ToString()</code>.
   */
  bool operator==(const LDAPFilter& other) const;

  LDAPFilter& operator=(const LDAPFilter& filter);

protected:

  SharedDataPointer<LDAPFilterData> d;

};

US_END_NAMESPACE

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/**
 * \ingroup MicroServices
 */
US_EXPORT std::ostream& operator<<(std::ostream& os, const US_PREPEND_NAMESPACE(LDAPFilter)& filter);

#endif // USLDAPFILTER_H
