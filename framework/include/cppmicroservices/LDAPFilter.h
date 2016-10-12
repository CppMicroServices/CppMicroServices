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

#ifndef CPPMICROSERVICES_LDAPFILTER_H
#define CPPMICROSERVICES_LDAPFILTER_H

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/SharedData.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace cppmicroservices {

class LDAPFilterData;
class ServiceReferenceBase;
class Bundle;

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
 *   - "(&(" + Constants::OBJECTCLASS + "=Person)(|(sn=Jensen)(cn=Babs J*)))"
 *   - "(o=univ*of*mich*)"
 *
 * \remarks This class is thread safe.
 *
 * \sa LDAPProp for a fluent API generating LDAP filter strings
 */
class US_Framework_EXPORT LDAPFilter {

private:

  typedef SharedDataPointer<LDAPFilterData> LDAPFilter::*bool_type;

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

  explicit operator bool() const;

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
  bool Match(const ServiceReferenceBase& reference) const;
    
 /**
   * Filter using a bundle's manifest properties.
   * <p>
   * This <code>LDAPFilter</code> is executed using the keys and values of the
   * bundle's manifest properties. The keys are looked up in a case insensitive 
   * manner.
   *
   * @param bundle The bundle whose properties are used
   *        in the match.
   * @return <code>true</code> if the bundle's properties match this
   *         <code>LDAPFilter</code> <code>false</code> otherwise.
   */
  bool Match(const Bundle& bundle) const;
  
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
  bool Match(const AnyMap& dictionary) const;

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
  bool MatchCase(const AnyMap& dictionary) const;

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

/**
 * \ingroup MicroServices
 */
US_Framework_EXPORT std::ostream& operator<<(std::ostream& os, const LDAPFilter& filter);

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // CPPMICROSERVICES_LDAPFILTER_H
