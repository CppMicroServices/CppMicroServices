// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_FLATTEN_HPP
#define JSONCONS_JSONPATH_FLATTEN_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::is_const
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <algorithm> // std::copy
#include <iterator> // std::back_inserter
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

namespace jsoncons { namespace jsonpath {

    template <class CharT, class Sink>
    std::size_t escape_string(const CharT* s, std::size_t length,
                              Sink& sink)
    {
        std::size_t count = 0;
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            switch (c)
            {
                case '\\':
                    sink.push_back('\\');
                    sink.push_back('\\');
                    count += 2;
                    break;
                case '\'':
                    sink.push_back('\\');
                    sink.push_back('\'');
                    count += 2;
                    break;
                case '\b':
                    sink.push_back('\\');
                    sink.push_back('b');
                    count += 2;
                    break;
                case '\f':
                    sink.push_back('\\');
                    sink.push_back('f');
                    count += 2;
                    break;
                case '\n':
                    sink.push_back('\\');
                    sink.push_back('n');
                    count += 2;
                    break;
                case '\r':
                    sink.push_back('\\');
                    sink.push_back('r');
                    count += 2;
                    break;
                case '\t':
                    sink.push_back('\\');
                    sink.push_back('t');
                    count += 2;
                    break;
                default:
                    sink.push_back(c);
                    ++count;
                    break;
            }
        }
        return count;
    }

    template<class Json>
    void flatten_(const std::basic_string<typename Json::char_type>& parent_key,
                  const Json& parent_value,
                  Json& result)
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        switch (parent_value.type())
        {
            case json_type::array_value:
            {
                if (parent_value.empty())
                {
                    result.try_emplace(parent_key, parent_value);
                }
                else
                {
                    for (std::size_t i = 0; i < parent_value.size(); ++i)
                    {
                        string_type key(parent_key);
                        key.push_back('[');
                        jsoncons::detail::from_integer(i,key);
                        key.push_back(']');
                        flatten_(key, parent_value.at(i), result);
                    }
                }
                break;
            }

            case json_type::object_value:
            {
                if (parent_value.empty())
                {
                    result.try_emplace(parent_key, Json());
                }
                else
                {
                    for (const auto& item : parent_value.object_range())
                    {
                        string_type key(parent_key);
                        key.push_back('[');
                        key.push_back('\'');
                        escape_string(item.key().data(), item.key().length(), key);
                        key.push_back('\'');
                        key.push_back(']');
                        flatten_(key, item.value(), result);
                    }
                }
                break;
            }

            default:
            {
                result[parent_key] = parent_value;
                break;
            }
        }
    }

    template<class Json>
    Json flatten(const Json& value)
    {
        Json result;
        std::basic_string<typename Json::char_type> parent_key = {'$'};
        flatten_(parent_key, value, result);
        return result;
    }

    enum class unflatten_state 
    {
        start,
        expect_left_bracket,
        left_bracket,
        single_quoted_name_state,
        double_quoted_name_state,
        index_state,
        expect_right_bracket,
        double_quoted_string_escape_char,
        single_quoted_string_escape_char
    };

    template<class Json>
    Json unflatten(const Json& value)
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        if (JSONCONS_UNLIKELY(!value.is_object()))
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::argument_to_unflatten_invalid));
        }

        Json result;

        for (const auto& item : value.object_range())
        {
            Json* part = &result;
            string_type buffer;
            unflatten_state state = unflatten_state::start;

            auto it = item.key().begin();
            auto last = item.key().end();

            for (; it != last; ++it)
            {
                switch (state)
                {
                    case unflatten_state::start:
                    {
                        switch (*it)
                        {
                            case '$':
                                state = unflatten_state::expect_left_bracket;
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case unflatten_state::expect_left_bracket:
                    {
                        switch (*it)
                        {
                            case '[':
                                state = unflatten_state::left_bracket;
                                break;
                            default:
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_flattened_key));
                                break;
                        }
                        break;
                    }
                    case unflatten_state::left_bracket:
                    {
                        switch (*it)
                        {
                            case '\'':
                                state = unflatten_state::single_quoted_name_state;
                                break;
                            case '\"':
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                buffer.push_back(*it);
                                state = unflatten_state::index_state;
                                break;
                            default:
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_flattened_key));
                                break;
                        }
                        break;
                    }
                    case unflatten_state::single_quoted_name_state:
                    {
                        switch (*it)
                        {
                            case '\'':
                                if (it != last-2)
                                {
                                    auto res = part->try_emplace(buffer,Json());
                                    part = &(res.first->value());
                                }
                                else
                                {
                                    auto res = part->try_emplace(buffer,item.value());
                                    part = &(res.first->value());
                                }
                                buffer.clear();
                                state = unflatten_state::expect_right_bracket;
                                break;
                            case '\\':
                                state = unflatten_state::single_quoted_string_escape_char;
                                break;
                            default:
                                buffer.push_back(*it);
                                break;
                        }
                        break;
                    }
                    case unflatten_state::double_quoted_name_state:
                    {
                        switch (*it)
                        {
                            case '\"':
                                if (it != last-2)
                                {
                                    auto res = part->try_emplace(buffer,Json());
                                    part = &(res.first->value());
                                }
                                else
                                {
                                    auto res = part->try_emplace(buffer,item.value());
                                    part = &(res.first->value());
                                }
                                buffer.clear();
                                state = unflatten_state::expect_right_bracket;
                                break;
                            case '\\':
                                state = unflatten_state::double_quoted_string_escape_char;
                                break;
                            default:
                                buffer.push_back(*it);
                                break;
                        }
                        break;
                    }
                    case unflatten_state::double_quoted_string_escape_char:
                        switch (*it)
                        {
                            case '\"':
                                buffer.push_back('\"');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case '\\': 
                                buffer.push_back('\\');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case '/':
                                buffer.push_back('/');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'b':
                                buffer.push_back('\b');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'f':
                                buffer.push_back('\f');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'n':
                                buffer.push_back('\n');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'r':
                                buffer.push_back('\r');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 't':
                                buffer.push_back('\t');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            default:
                                break;
                        }
                        break;
                    case unflatten_state::single_quoted_string_escape_char:
                        switch (*it)
                        {
                            case '\'':
                                buffer.push_back('\'');
                                state = unflatten_state::single_quoted_name_state;
                                break;
                            case '\\': 
                                buffer.push_back('\\');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case '/':
                                buffer.push_back('/');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'b':
                                buffer.push_back('\b');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'f':
                                buffer.push_back('\f');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'n':
                                buffer.push_back('\n');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 'r':
                                buffer.push_back('\r');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            case 't':
                                buffer.push_back('\t');
                                state = unflatten_state::double_quoted_name_state;
                                break;
                            default:
                                break;
                        }
                        break;
                    case unflatten_state::index_state:
                    {
                        switch (*it)
                        {
                            case ']':
                            {
                                auto r = jsoncons::detail::to_integer<size_t>(buffer.data(), buffer.size());
                                if (r)
                                {
                                    if (!part->is_array())
                                    {
                                        *part = Json(json_array_arg);
                                    }
                                    if (it != last-1)
                                    {
                                        if (r.value()+1 > part->size())
                                        {
                                            Json& ref = part->emplace_back();
                                            part = std::addressof(ref);
                                        }
                                        else
                                        {
                                            part = &part->at(r.value());
                                        }
                                    }
                                    else
                                    {
                                        Json& ref = part->emplace_back(item.value());
                                        part = std::addressof(ref);
                                    }
                                }
                                buffer.clear();
                                state = unflatten_state::expect_left_bracket;
                                break;
                            }
                            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                buffer.push_back(*it);
                                break;
                            default:
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_flattened_key));
                                break;
                        }
                        break;
                    }
                    case unflatten_state::expect_right_bracket:
                    {
                        switch (*it)
                        {
                            case ']':
                                state = unflatten_state::expect_left_bracket;
                                break;
                            default:
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_flattened_key));
                                break;
                        }
                        break;
                    }
                    default:
                        JSONCONS_UNREACHABLE();
                        break;
                }
            }
        }

        return result;
    }
}}

#endif
