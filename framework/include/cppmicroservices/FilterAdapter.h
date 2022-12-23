/* TODO: Copyright */

#ifndef FILTER_ADAPTER_H
#define FILTER_ADAPTER_H
#pragma once

#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/JSONFilter.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/ServiceReferenceBase.h"

namespace cppmicroservices
{

    using ObjectClassSet = std::unordered_set<std::string>;
    using StringList = std::vector<std::string>;
    using LocalCache = std::vector<StringList>;

    class US_Framework_EXPORT FilterAdapter
    {
      public:
        struct AlwaysMatchFilter
        {
        };

        FilterAdapter() = default;
        FilterAdapter(FilterAdapter const&) = default;
        FilterAdapter(std::string filter_string);
        FilterAdapter(char const* filter_string);
        FilterAdapter(LDAPFilter f);
        FilterAdapter(JSONFilter f);

        /**
         * Filter using a service's properties.
         *
         * @param reference The reference to the service whose properties are used
         *        in the match.
         * @return <code>true</code> if the service's properties match this filter
         */
        bool Match(ServiceReferenceBase const& reference) const;

        /**
         * Filter using a bundle's manifest headers.
         *
         * @param bundle The bundle whose manifest's headers are used
         *        in the match.
         * @return <code>true</code> if the bundle's manifest headers match this filter
         */
        bool Match(Bundle const& bundle) const;
        bool Match(AnyMap const& props) const;

        bool IsComplicated() const;
        bool AddToSimpleCache(StringList const& keywords, LocalCache& cache) const;

        std::string ToString() const;

        bool GetMatchedObjectClasses(ObjectClassSet& matches) const;

        bool IsLDAPFilter() const;
        bool IsJSONFilter() const;
        
        FilterAdapter& operator=(FilterAdapter const& other);
        FilterAdapter& operator=(LDAPFilter const& other);
        FilterAdapter& operator=(JSONFilter const& other);
        FilterAdapter& operator=(std::string const& filter_string);
        FilterAdapter& operator=(char* const filter_string);

      private:
        static bool isLDAPFilterString(std::string const& filter_string);

        std::variant<cppmicroservices::LDAPFilter, cppmicroservices::JSONFilter, AlwaysMatchFilter> filter;
    };

} // namespace cppmicroservices
#endif // FILTER_ADAPTER_H
