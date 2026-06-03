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

#include "Properties.h"

#include <limits>
#include <stdexcept>
#include <utility>

#include "PropsCheck.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

namespace cppmicroservices
{

    const Any Properties::emptyAny;

    void
    Properties::PopulateCaseInsensitiveLookupMap() const
    {
        if (caseInsensitiveLookup.size() >= props.size())
        {
            return;
        }

        if (props.GetType() == AnyMap::ORDERED_MAP)
        {
            auto const& om = props.get<AnyMap::ordered_any_map>();
            for (auto const& itr : om)
            {
                caseInsensitiveLookup.insert(itr.first);
            }
        }
        else if (props.GetType() == AnyMap::UNORDERED_MAP)
        {
            auto const& uom = props.get<AnyMap::unordered_any_map>();
            for (auto const& itr : uom)
            {
                caseInsensitiveLookup.insert(itr.first);
            }
        }
        else
        {
            throw std::runtime_error("Unsupported map type.");
        }
    }

    // NOTE: UNORDERED_MAP_CASEINSENSITIVE_KEYS AnyMaps inherently can never be invalid given that
    // they can _never_ contain some pair of keys which are only different in case. Because of this,
    // the validation check is not performed on these map types. Additionally, those types of maps
    // are already case insensitive so populating and using the case insensitive lookup map is
    // unnecessary.

    Properties::Properties(AnyMap const& p) : props(p)
    {
        if (p.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
        {
            props_check::ValidateAnyMap(p);
        }
    }

    Properties::Properties(AnyMap&& p) : props(std::move(p))
    {
        if (props.GetType() != AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
        {
            props_check::ValidateAnyMap(props);
        }
    }

    Properties::Properties(Properties&& o) noexcept : props(std::move(o.props)) {}

    Properties&
    Properties::operator=(Properties&& o) noexcept
    {
        props = std::move(o.props);
        caseInsensitiveLookup = std::move(o.caseInsensitiveLookup);

        return *this;
    }

    Any const&
    Properties::ValueByRef_unlocked(std::string const& key, bool matchCase) const
    {
        if (props.GetType() == AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
        {
            auto const& cimap = props.get<AnyMap::unordered_any_cimap>();
            if (auto itr = cimap.find(key); itr != cimap.end())
            {
                if (!matchCase)
                {
                    return itr->second;
                }
                else if (matchCase && itr->first == key)
                {
                    return itr->second;
                }
                else
                {
                    return emptyAny;
                }
            }
            else
            {
                return emptyAny;
            }
        }
        else if (props.GetType() == AnyMap::UNORDERED_MAP)
        {
            auto const& uom = props.get<AnyMap::unordered_any_map>();
            auto itr = uom.find(key);
            if (itr != uom.end())
            {
                return itr->second;
            }

            if (!matchCase)
            {
                PopulateCaseInsensitiveLookupMap();

                auto ciItr = caseInsensitiveLookup.find(key);
                if (ciItr != caseInsensitiveLookup.end())
                {
                    return uom.find(*ciItr)->second;
                }
                else
                {
                    return emptyAny;
                }
            }

            return emptyAny;
        }
        else if (props.GetType() == AnyMap::ORDERED_MAP)
        {
            auto const& om = props.get<AnyMap::ordered_any_map>();
            auto itr = om.find(key);
            if (itr != om.end())
            {
                return itr->second;
            }

            if (!matchCase)
            {
                PopulateCaseInsensitiveLookupMap();

                auto ciItr = caseInsensitiveLookup.find(key);
                if (ciItr != caseInsensitiveLookup.end())
                {
                    return om.find(*ciItr)->second;
                }
                else
                {
                    return emptyAny;
                }
            }

            return emptyAny;
        }
        else
        {
            throw std::runtime_error("Unknown AnyMap type.");
        }
    }

    std::pair<Any, bool>
    Properties::Value_unlocked(std::string const& key, bool matchCase) const
    {
        if (props.GetType() == AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS)
        {
            auto const& cimap = props.get<AnyMap::unordered_any_cimap>();
            if (auto itr = cimap.find(key); itr != cimap.end())
            {
                if (!matchCase)
                {
                    return std::make_pair(itr->second, true);
                }
                else if (matchCase && itr->first == key)
                {
                    return std::make_pair(itr->second, true);
                }
                else
                {
                    return std::make_pair(emptyAny, false);
                }
            }
            else
            {
                return std::make_pair(emptyAny, false);
            }
        }
        else if (props.GetType() == AnyMap::UNORDERED_MAP)
        {
            auto const& uom = props.get<AnyMap::unordered_any_map>();
            auto itr = uom.find(key);
            if (itr != uom.end())
            {
                return std::make_pair(itr->second, true);
            }

            if (!matchCase)
            {
                PopulateCaseInsensitiveLookupMap();

                auto ciItr = caseInsensitiveLookup.find(key);
                if (ciItr != caseInsensitiveLookup.end())
                {
                    return std::make_pair(uom.find(*ciItr)->second, true);
                }
                else
                {
                    return std::make_pair(emptyAny, false);
                }
            }

            return std::make_pair(emptyAny, false);
        }
        else if (props.GetType() == AnyMap::ORDERED_MAP)
        {
            auto const& om = props.get<AnyMap::ordered_any_map>();
            auto itr = om.find(key);
            if (itr != om.end())
            {
                return std::make_pair(itr->second, true);
            }

            if (!matchCase)
            {
                PopulateCaseInsensitiveLookupMap();

                auto ciItr = caseInsensitiveLookup.find(key);
                if (ciItr != caseInsensitiveLookup.end())
                {
                    return std::make_pair(om.find(*ciItr)->second, true);
                }
                else
                {
                    return std::make_pair(emptyAny, false);
                }
            }

            return std::make_pair(emptyAny, false);
        }
        else
        {
            throw std::runtime_error("Unknown AnyMap type.");
        }
    }

    std::vector<std::string>
    Properties::Keys_unlocked() const
    {
        std::vector<std::string> result {};
        for (auto const& kv_pair : props)
        {
            result.push_back(kv_pair.first);
        }

        return result;
    }

    void
    Properties::Clear_unlocked()
    {
        props.clear();
    }
} // namespace cppmicroservices

US_MSVC_POP_WARNING
