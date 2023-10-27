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

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4251)
#endif

namespace cppmicroservices
{

    class LDAPFilterData;
    class ServiceReferenceBase;
    class Bundle;

    /**
    \defgroup gr_ldap LDAP Filter

    \brief Groups LDAPFilter class related symbols.
    */

    /**
     * \ingroup MicroServices
     * \ingroup gr_ldap
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
     * LDAPFilters have been extended to make it easier to query nested JSON keys. This is disabled
     * by default. To enable this functionality, define SUPPORT_NESTED_LOOKUP at compile time. For
     * example, if using "make" to build:
     *
     * >> make clean
     * >> make SUPPORT_NESTED_LOOKUP=1
     *
     * Nested Lookup Description
     * =========================
     *
     * Keys which contain the "." character may refer to nested values. The top level is checked
     * first for a matching entry. If one isn't found, the key is decomposed and the JSON structure
     * "walked down" to look for a match.
     *
     * For example, given a key "a.b.c.d", if a value exists in the top level map with that key,
     * it's retuned. If a value is not found at the top level with that key, it's decomposed into a
     * vector: ["a","b","c","d"]. Ultimately for the filter to be applied, there must be a value in
     * a nested AnyMap with a key ending with "d". So, the top level map is checked for the key
     * "a". If a value rather than a map is found there, there is no path to "d", so the algorithm
     * stops with an unsuccessful lookup. If a map does exist, the algorithm continues and looks for
     * a key of "b" in that map and continues from there with the same algorithm (looking at that
     * submap for keys "b", "b.c", and "b.c.d"). Finally, if nothing has been found for key "a",
     * the algorithm combines the first two keys together into "a.b" and looks for a submap.
     * Again, if a value is found rather than a map, there is no path to "d" so we halt the
     * lookup. If a submap is found there, we then look in that map for a key of "c" and continue
     * from there. Finally, if there is no item in the to plevel map at "a.b", we look at
     * "a.b.c". And again, a value there halts the algorithm. If a map is found there, it's checked
     * for a value at key "d".
     *
     * A real world example:
     *
     *   - "("bundle.symbolic_name=my_bundle)". This will look for a match in two places. First it
     *     will look in the JSON manifest for:
     *     { "bundle.symbolic_name" : "my_bundle" }
     *
     *     If not found there, this will be checked:
     *     { "bundle" : { "symbolic_name" : "my_bundle" } }
     *   - top level flat keys are preferred in order to preserve the behavior of existing filters.
     *
     * \remarks This class is thread safe.
     *
     * \sa Use LDAPProp class to conveniently generate LDAP filter strings
     *
     */
    class US_Framework_EXPORT LDAPFilter
    {

      public:
        /**
         * Creates a valid <code>LDAPFilter</code> object that
         * matches nothing.
         *
         */
        LDAPFilter();

        /**
         * Creates a <code>LDAPFilter</code> object encapsulating the filter string.
         * This <code>LDAPFilter</code> object may be used to match a
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
        LDAPFilter(std::string const& filter);

        LDAPFilter(LDAPFilter const& other);

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
        bool Match(ServiceReferenceBase const& reference) const;

        /**
         * Filter using a bundle's manifest headers.
         * <p>
         * This <code>LDAPFilter</code> is executed using the keys and values of the
         * bundle's manifest headers. The keys are looked up in a case insensitive
         * manner.
         *
         * @param bundle The bundle whose manifest's headers are used
         *        in the match.
         * @return <code>true</code> if the bundle's manifest headers match this
         *         <code>LDAPFilter</code> <code>false</code> otherwise.
         * @throws std::runtime_error If the number of keys of the bundle's manifest
         *         headers exceeds the value returned by std::numeric_limits<int>::max().
         */
        bool Match(Bundle const& bundle) const;

        /**
         * Filter using a <code>AnyMap</code> object with case insensitive key lookup. This
         * <code>LDAPFilter</code> is executed using the specified <code>AnyMap</code>'s keys
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
        bool Match(AnyMap const& dictionary) const;

        /**
         * Filter using a <code>AnyMap</code>. This <code>LDAPFilter</code> is executed using
         * the specified <code>AnyMap</code>'s keys and values. The keys are looked
         * up in a normal manner respecting case.
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
        bool MatchCase(AnyMap const& dictionary) const;

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
        bool operator==(LDAPFilter const& other) const;

        LDAPFilter& operator=(LDAPFilter const& filter);

      protected:
        std::shared_ptr<LDAPFilterData> d;
    };

    /**
     * \ingroup MicroServices
     * \ingroup gr_ldap
     *
     * Streams the string representation of \c filter into the stream \c os
     * via LDAPFilter::ToString().
     */
    US_Framework_EXPORT std::ostream& operator<<(std::ostream& os, LDAPFilter const& filter);
} // namespace cppmicroservices

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

#endif // CPPMICROSERVICES_LDAPFILTER_H
