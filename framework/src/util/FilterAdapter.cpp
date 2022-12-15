/* TODO: Copyright */

#include "cppmicroservices/FilterAdapter.h"

namespace cppmicroservices {
  
  namespace {
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
      bool operator()(const FilterAdapter::AlwaysMatchFilter&) const { return true; }

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
      bool operator()(const FilterAdapter::AlwaysMatchFilter&) const { return false; }
    
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
      bool operator()(const FilterAdapter::AlwaysMatchFilter&) const { return false; }
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
      bool operator()(const FilterAdapter::AlwaysMatchFilter&) const { return false; }
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
      std::string operator()(const FilterAdapter::AlwaysMatchFilter&) const { return ""; }
    };
  }

  FilterAdapter::FilterAdapter(std::string filter_string)
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

  FilterAdapter::FilterAdapter(const char* filter_string)
    : filter()
  {
    FilterAdapter(std::string(filter_string));
  }

  FilterAdapter::FilterAdapter(LDAPFilter f)
    : filter(std::move(f))
  {
  }

  FilterAdapter::FilterAdapter(JSONFilter f)
    : filter(std::move(f))
  {
  }

  bool FilterAdapter::Match(const ServiceReferenceBase& reference) const
  {
    return std::visit(MatchVisitor(reference), filter);
  }

  bool FilterAdapter::Match(const Bundle& bundle) const
  {
    return std::visit(MatchVisitor{bundle}, filter);
  }

  bool FilterAdapter::Match(const AnyMap& props) const
  {
    return std::visit(MatchVisitor{props}, filter);
  }

  bool FilterAdapter::IsComplicated() const
  {
    return std::visit(IsComplicatedVisitor{}, filter);
  }

  bool FilterAdapter::AddToSimpleCache(const StringList& keywords, LocalCache& cache) const
  {
    return std::visit(AddToSimpleCacheVisitor{keywords,cache},filter);
  }

  std::string FilterAdapter::ToString() const {
    return std::visit(ToStringVisitor{},filter);
  }

  bool FilterAdapter::GetMatchedObjectClasses(ObjectClassSet& matches) const
  {
    return std::visit(MatchedObjectClassesVisitor{matches},filter);
  }

  FilterAdapter& FilterAdapter::operator=(const FilterAdapter& other)
  {
    if (this != &other) {
      filter = other.filter;
    }
    return *this;
  }
  FilterAdapter& FilterAdapter::operator=(const LDAPFilter& other)
  {
    filter = other;
    return *this;
  }
  FilterAdapter& FilterAdapter::operator=(const JSONFilter& other)
  {
    filter = other;
    return *this;
  }

  FilterAdapter& FilterAdapter::operator=(std::string const& filter_string)
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
    return *this;
  }

  FilterAdapter& FilterAdapter::operator=(char* const filter_string)
  {
    return operator=(std::string(filter_string));
  }

  bool FilterAdapter::isLDAPFilterString(const std::string& filter_string)
  {
    /* parse f and figure out what kind of filter it represents, and then create a correct filter
       object and assign it into filter */
    return (!filter_string.empty() && ('(' == filter_string.at(0)));
  }

} // namespace cppmicroservices
