/* TODO: Copyright */

#include "cppmicroservices/FilterAdapter.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"

namespace cppmicroservices
{

    namespace
    {
        // T is one of ServiceReferenceBase or Bundle
        template <typename T>
        class MatchVisitor
        {
          public:
            MatchVisitor(T const& r) : ref(r) {}

            bool
            operator()(cppmicroservices::LDAPFilter const& filter) const
            {
                return filter.Match(ref);
            }
            bool
            operator()(cppmicroservices::JSONFilter const& filter) const
            {
                return filter.Match(ref);
            }
            bool
            operator()(FilterAdapter::AlwaysMatchFilter const&) const
            {
                return true;
            }

          private:
            T const& ref;
        };

        class MatchedObjectClassesVisitor
        {
          public:
            MatchedObjectClassesVisitor(ObjectClassSet& matches) : matchedClasses(matches) {}

            bool
            operator()(cppmicroservices::LDAPFilter const& filter) const
            {
                return filter.GetMatchedObjectClasses(matchedClasses);
            }

            template <typename T>
            bool
            operator()(T const&) const
            {
                return false;
            }

          private:
            ObjectClassSet& matchedClasses;
        };

        class AddToSimpleCacheVisitor
        {
          public:
            AddToSimpleCacheVisitor(StringList const& k, LocalCache& c) : keywords(k), cache(c) {}

            bool
            operator()(cppmicroservices::LDAPFilter const& filter) const
            {
                if (!filter.IsComplicated())
                {
                    return filter.AddToSimpleCache(keywords, cache);
                }
                return false;
            }

            template <typename T>
            bool
            operator()(T const&) const
            {
                return false;
            }

          private:
            StringList const& keywords;
            LocalCache& cache;
        };

        class IsComplicatedVisitor
        {
          public:
            bool
            operator()(cppmicroservices::LDAPFilter const& filter) const
            {
                return filter.IsComplicated();
            }

            template <typename T>
            bool
            operator()(T const&) const
            {
                return false;
            }
        };

        class IsLDAPFilterVisitor
        {
          public:
            bool
            operator()(cppmicroservices::LDAPFilter const&) const
            {
                return true;
            }

            template <typename T>
            bool
            operator()(T const&) const
            {
                return false;
            }
        };

        class IsJSONFilterVisitor
        {
          public:
            bool
            operator()(cppmicroservices::JSONFilter const&) const
            {
                return true;
            }

            template <typename T>
            bool
            operator()(T const&) const
            {
                return false;
            }
        };

        class ToStringVisitor
        {
          public:
            std::string
            operator()(cppmicroservices::LDAPFilter const& filter) const
            {
                return filter.ToString();
            }
            std::string
            operator()(cppmicroservices::JSONFilter const& filter) const
            {
                return filter.ToString();
            }
            std::string
            operator()(FilterAdapter::AlwaysMatchFilter const&) const
            {
                return "";
            }
        };
    } // namespace

    FilterAdapter::FilterAdapter(std::string filter_string) : filter()
    {
        if (filter_string.empty())
        {
            filter = AlwaysMatchFilter {};
        }
        else if (isLDAPFilterString(filter_string))
        {
            filter = cppmicroservices::LDAPFilter(std::move(filter_string));
        }
        else
        {
            filter = cppmicroservices::JSONFilter(std::move(filter_string));
        }
    }

    FilterAdapter::FilterAdapter(char const* filter_string) : filter() { FilterAdapter(std::string(filter_string)); }

    FilterAdapter::FilterAdapter(LDAPFilter f) : filter(std::move(f)) {}

    FilterAdapter::FilterAdapter(JSONFilter f) : filter(std::move(f)) {}

    bool
    FilterAdapter::Match(ServiceReferenceBase const& reference) const
    {
        return std::visit(MatchVisitor(reference), filter);
    }

    bool
    FilterAdapter::Match(Bundle const& bundle) const
    {
        return std::visit(MatchVisitor { bundle }, filter);
    }

    bool
    FilterAdapter::Match(AnyMap const& props) const
    {
        return std::visit(MatchVisitor { props }, filter);
    }

    bool
    FilterAdapter::IsComplicated() const
    {
        return std::visit(IsComplicatedVisitor {}, filter);
    }

    bool
    FilterAdapter::AddToSimpleCache(StringList const& keywords, LocalCache& cache) const
    {
        return std::visit(AddToSimpleCacheVisitor { keywords, cache }, filter);
    }

    std::string
    FilterAdapter::ToString() const
    {
        return std::visit(ToStringVisitor {}, filter);
    }

    bool
    FilterAdapter::GetMatchedObjectClasses(ObjectClassSet& matches) const
    {
        return std::visit(MatchedObjectClassesVisitor { matches }, filter);
    }

    bool FilterAdapter::IsLDAPFilter() const
    {
        return std::visit(IsLDAPFilterVisitor {}, filter);
    }
        
    bool FilterAdapter::IsJSONFilter() const
    {
        return std::visit(IsJSONFilterVisitor {}, filter);
    }
        
    FilterAdapter&
    FilterAdapter::operator=(FilterAdapter const& other)
    {
        if (this != &other)
        {
            filter = other.filter;
        }
        return *this;
    }
    FilterAdapter&
    FilterAdapter::operator=(LDAPFilter const& other)
    {
        filter = other;
        return *this;
    }
    FilterAdapter&
    FilterAdapter::operator=(JSONFilter const& other)
    {
        filter = other;
        return *this;
    }

    FilterAdapter&
    FilterAdapter::operator=(std::string const& filter_string)
    {
        if (filter_string.empty())
        {
            filter = AlwaysMatchFilter {};
        }
        else if (isLDAPFilterString(filter_string))
        {
            filter = cppmicroservices::LDAPFilter(std::move(filter_string));
        }
        else
        {
            filter = cppmicroservices::JSONFilter(std::move(filter_string));
        }
        return *this;
    }

    FilterAdapter&
    FilterAdapter::operator=(char* const filter_string)
    {
        return operator=(std::string(filter_string));
    }

    bool
    FilterAdapter::isLDAPFilterString(std::string const& filter_string)
    {
        /* parse f and figure out what kind of filter it represents, and then create a correct filter
           object and assign it into filter */
        try
        {
            cppmicroservices::LDAPFilter filter(filter_string);
        }
        catch (std::invalid_argument e)
        {
            return false;
        }
        return true;
    }

} // namespace cppmicroservices
