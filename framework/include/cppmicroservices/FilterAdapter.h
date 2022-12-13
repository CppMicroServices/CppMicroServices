/* TODO: Copyright */

#ifndef FILTERING_STRATEGY_H
#define FILTERING_STRATEGY_H
#pragma once

#include <string>
#include <variant>
#include <vector>
#include <unordered_set>

#include "cppmicroservices/JSONFilter.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReferenceBase.h"
#include "cppmicroservices/AnyMap.h"

namespace cppmicroservices {

class FilterAdapter
{
public:
  using ObjectClassSet = std::unordered_set<std::string>;
  using StringList = std::vector<std::string>;
  using LocalCache = std::vector<StringList>;

private:
  struct AlwaysMatchFilter {};

  // T is one of ServiceReferenceBase or Bundle
  template<typename T>
  class MatchVisitor
  {
  public:
    MatchVisitor(const T& r)
      : ref(r)
    {
    }

    bool operator()(const cppmicroservices::LDAPFilter& filter) const
    {
      return filter.Match(ref);
    }
    bool operator()(const cppmicroservices::JSONFilter& filter) const
    {
      return filter.Match(ref);
    }
    bool operator()(const AlwaysMatchFilter&) const { return true; }

  private:
    const T& ref;
  };

  class MatchedObjectClassesVisitor
  {
  public:
    MatchedObjectClassesVisitor(ObjectClassSet& matches)
      : matchedClasses(matches)
    {
    }

    bool operator()(const cppmicroservices::LDAPFilter& filter) const
    {
      return filter.GetMatchedObjectClasses(matchedClasses);
    }
    bool operator()(const cppmicroservices::JSONFilter&) const { return false; }
    bool operator()(const AlwaysMatchFilter&) const { return false; }
    
  private:
    ObjectClassSet& matchedClasses;
  };
  
  class AddToSimpleCacheVisitor
  {
  public:
    AddToSimpleCacheVisitor(const StringList& k, LocalCache& c)
      : keywords(k), cache(c)
    {
    }

    bool operator()(const cppmicroservices::LDAPFilter& filter) const
    {
      if (!filter.IsComplicated()) {
        return filter.AddToSimpleCache(keywords,cache);
      }
      return false;
    }
    bool operator()(const cppmicroservices::JSONFilter&) const { return false; }
    bool operator()(const AlwaysMatchFilter&) const { return false; }
  private:
    const StringList& keywords;
    LocalCache& cache;
  };

  class IsComplicatedVisitor
  {
  public:
    bool operator()(const cppmicroservices::LDAPFilter& filter) const
    {
      return filter.IsComplicated();
    }
    bool operator()(const cppmicroservices::JSONFilter&) const { return true; }
    bool operator()(const AlwaysMatchFilter&) const { return false; }
  };


  class ToStringVisitor
  {
  public:
    std::string operator()(const cppmicroservices::LDAPFilter& filter) const
    {
      return filter.ToString();
    }
    std::string operator()(const cppmicroservices::JSONFilter& filter) const
    {
      return filter.ToString();
    }
    std::string operator()(const AlwaysMatchFilter&) const { return ""; }
  };


public:
  FilterAdapter() = default;
  FilterAdapter(const FilterAdapter&) = default;

  FilterAdapter(std::string filter_string)
    : filter()
  {
    if (filter_string.empty()) {
      filter = AlwaysMatchFilter {};
    }
    else if (isLDAPFilterString(filter_string)) {
      filter = cppmicroservices::LDAPFilter(std::move(filter_string));
    }
    else {
      filter = cppmicroservices::JSONFilter(std::move(filter_string));
    }
  }

  FilterAdapter(const char* filter_string)
    : filter()
  {
    FilterAdapter(std::string(filter_string));
  }

  template<typename FilterT>
  FilterAdapter(FilterT f)
    : filter(std::move(f))
  {
  }

  /**
     * Filter using a service's properties.
     *
     * @param reference The reference to the service whose properties are used
     *        in the match.
     * @return <code>true</code> if the service's properties match this filter
     */
  bool Match(const ServiceReferenceBase& reference) const
  {
    return std::visit(MatchVisitor(reference), filter);
  }

  /**
     * Filter using a bundle's manifest headers.
     *
     * @param bundle The bundle whose manifest's headers are used
     *        in the match.
     * @return <code>true</code> if the bundle's manifest headers match this filter
     */
  bool Match(const Bundle& bundle) const
  {
    return std::visit(MatchVisitor{bundle}, filter);
  }

  bool Match(const AnyMap& props) const
  {
    return std::visit(MatchVisitor{props}, filter);
  }

  bool IsComplicated() const
  {
    return std::visit(IsComplicatedVisitor{}, filter);
  }

  bool AddToSimpleCache(const StringList& keywords, LocalCache& cache) const
  {
    return std::visit(AddToSimpleCacheVisitor{keywords,cache},filter);
  }

  std::string ToString() const {
    return std::visit(ToStringVisitor{},filter);
  }

  bool GetMatchedObjectClasses(ObjectClassSet& matches) const
  {
    return std::visit(MatchedObjectClassesVisitor{matches},filter);
  }

  FilterAdapter& operator=(const FilterAdapter& other)
  {
    filter = other.filter;
    return *this;
  }
  FilterAdapter& operator=(const LDAPFilter& other)
  {
    filter = other;
    return *this;
  }
  FilterAdapter& operator=(const JSONFilter& other)
  {
    filter = other;
    return *this;
  }

private:
  static bool isLDAPFilterString(const std::string& filter_string)
  {
    /* parse f and figure out what kind of filter it represents, and then create a correct filter
         object and assign it into filter */
    return (!filter_string.empty() && ('(' == filter_string.at(0)));
  }

  std::variant<cppmicroservices::LDAPFilter, cppmicroservices::JSONFilter, AlwaysMatchFilter> filter;
};

} // namespace cppmicroservices
#endif
