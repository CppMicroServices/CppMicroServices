// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPATCH_HPP
#define JSONCONS_JSONPOINTER_JSONPATCH_HPP

#include <string>
#include <vector> 
#include <memory>
#include <algorithm> // std::min
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch_error.hpp>

namespace jsoncons { namespace jsonpatch {

namespace detail {

    template<class Json>
    std::basic_string<typename Json::char_type> normalized_path(const Json& root, const typename Json::string_view_type& location)
    {
        using string_type = std::basic_string<typename Json::char_type>;
        using string_view_type = typename Json::string_view_type;

        std::error_code ec;

        const Json* current = std::addressof(root);
        if (location.empty())
        {
            return string_type();
        }

        string_type buffer;
        jsonpointer::json_pointer_iterator<typename string_view_type::iterator> it(location.begin(), location.end());
        jsonpointer::json_pointer_iterator<typename string_view_type::iterator> end(location.begin(), location.end(), location.end());
        while (it != end)
        {
            buffer = *it;
            it.increment(ec);
            if (ec)
                return string_type(location);
            if (it != end)
            {
                current = jsoncons::jsonpointer::detail::resolve(current, buffer, ec);
                if (ec)
                    return string_type(location);
            }
        }

        if (current->is_array() && buffer.size() == 1 && buffer[0] == '-')
        {
            string_type p = string_type(location.substr(0,location.length()-1));
            std::string s = std::to_string(current->size());
            for (auto c : s)
            {
                p.push_back(c);
            }
            return p;
        }
        else
        {
            return string_type(location);
        }
    }

    JSONCONS_STRING_LITERAL(test_literal,'t','e','s','t')
    JSONCONS_STRING_LITERAL(add_literal,'a','d','d')
    JSONCONS_STRING_LITERAL(remove_literal,'r','e','m','o','v','e')
    JSONCONS_STRING_LITERAL(replace_literal,'r','e','p','l','a','c','e')
    JSONCONS_STRING_LITERAL(move_literal,'m','o','v','e')
    JSONCONS_STRING_LITERAL(copy_literal,'c','o','p','y')
    JSONCONS_STRING_LITERAL(op_literal,'o','p')
    JSONCONS_STRING_LITERAL(path_literal,'p','a','t','h')
    JSONCONS_STRING_LITERAL(from_literal,'f','r','o','m')
    JSONCONS_STRING_LITERAL(value_literal,'v','a','l','u','e')

    enum class op_type {add,remove,replace};
    enum class state_type {begin,abort,commit};

    template <class Json>
    struct operation_unwinder
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using string_view_type = typename Json::string_view_type;

        struct entry
        {
            op_type op;
            string_type path;
            Json value;

            entry(op_type op, const string_type& path, const Json& value)
                : op(op), path(path), value(value)
            {
            }

            entry(const entry&) = default;

            entry(entry&&) = default;

            entry& operator=(const entry&) = default;

            entry& operator=(entry&&) = default;
        };

        Json& target;
        state_type state;
        std::vector<entry> stack;

        operation_unwinder(Json& j)
            : target(j), state(state_type::begin)
        {
        }

        ~operation_unwinder() noexcept
        {
            std::error_code ec;
            //std::cout << "state: " << std::boolalpha << (state == state_type::commit) << ", stack size: " << stack.size() << std::endl;
            if (state != state_type::commit)
            {
                for (auto it = stack.rbegin(); it != stack.rend(); ++it)
                {
                    if (it->op == op_type::add)
                    {
                        jsonpointer::add(target,it->path,it->value,ec);
                        if (ec)
                        {
                            //std::cout << "add: " << it->path << std::endl;
                            break;
                        }
                    }
                    else if (it->op == op_type::remove)
                    {
                        jsonpointer::remove(target,it->path,ec);
                        if (ec)
                        {
                            //std::cout << "remove: " << it->path << std::endl;
                            break;
                        }
                    }
                    else if (it->op == op_type::replace)
                    {
                        jsonpointer::replace(target,it->path,it->value,ec);
                        if (ec)
                        {
                            //std::cout << "replace: " << it->path << std::endl;
                            break;
                        }
                    }
                }
            }
        }
    };

    template <class Json>
    Json from_diff(const Json& source, const Json& target, const typename Json::string_view_type& path)
    {
        using char_type = typename Json::char_type;

        Json result = typename Json::array();

        if (source == target)
        {
            return result;
        }

        if (source.is_array() && target.is_array())
        {
            std::size_t common = (std::min)(source.size(),target.size());
            for (std::size_t i = 0; i < common; ++i)
            {
                std::basic_string<char_type> ss(path); 
                ss.push_back('/');
                jsoncons::detail::from_integer(i,ss);
                auto temp_diff = from_diff(source[i],target[i],ss);
                result.insert(result.array_range().end(),temp_diff.array_range().begin(),temp_diff.array_range().end());
            }
            // Element in source, not in target - remove
            for (std::size_t i = source.size(); i-- > target.size();)
            {
                std::basic_string<char_type> ss(path); 
                ss.push_back('/');
                jsoncons::detail::from_integer(i,ss);
                Json val(json_object_arg);
                val.insert_or_assign(op_literal<char_type>(), remove_literal<char_type>());
                val.insert_or_assign(path_literal<char_type>(), ss);
                result.push_back(std::move(val));
            }
            // Element in target, not in source - add, 
            // Fix contributed by Alexander rog13
            for (std::size_t i = source.size(); i < target.size(); ++i)
            {
                const auto& a = target[i];
                std::basic_string<char_type> ss(path); 
                ss.push_back('/');
                jsoncons::detail::from_integer(i,ss);
                Json val(json_object_arg);
                val.insert_or_assign(op_literal<char_type>(), add_literal<char_type>());
                val.insert_or_assign(path_literal<char_type>(), ss);
                val.insert_or_assign(value_literal<char_type>(), a);
                result.push_back(std::move(val));
            }
        }
        else if (source.is_object() && target.is_object())
        {
            for (const auto& a : source.object_range())
            {
                std::basic_string<char_type> ss(path);
                ss.push_back('/'); 
                jsonpointer::escape(a.key(),ss);
                auto it = target.find(a.key());
                if (it != target.object_range().end())
                {
                    auto temp_diff = from_diff(a.value(),it->value(),ss);
                    result.insert(result.array_range().end(),temp_diff.array_range().begin(),temp_diff.array_range().end());
                }
                else
                {
                    Json val(json_object_arg);
                    val.insert_or_assign(op_literal<char_type>(), remove_literal<char_type>());
                    val.insert_or_assign(path_literal<char_type>(), ss);
                    result.push_back(std::move(val));
                }
            }
            for (const auto& a : target.object_range())
            {
                auto it = source.find(a.key());
                if (it == source.object_range().end())
                {
                    std::basic_string<char_type> ss(path); 
                    ss.push_back('/');
                    jsonpointer::escape(a.key(),ss);
                    Json val(json_object_arg);
                    val.insert_or_assign(op_literal<char_type>(), add_literal<char_type>());
                    val.insert_or_assign(path_literal<char_type>(), ss);
                    val.insert_or_assign(value_literal<char_type>(), a.value());
                    result.push_back(std::move(val));
                }
            }
        }
        else
        {
            Json val(json_object_arg);
            val.insert_or_assign(op_literal<char_type>(), replace_literal<char_type>());
            val.insert_or_assign(path_literal<char_type>(), path);
            val.insert_or_assign(value_literal<char_type>(), target);
            result.push_back(std::move(val));
        }

        return result;
    }
}

template <class Json>
void apply_patch(Json& target, const Json& patch, std::error_code& patch_ec)
{
    using char_type = typename Json::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = typename Json::string_view_type;

   jsoncons::jsonpatch::detail::operation_unwinder<Json> unwinder(target);

    // Validate
    
    string_type bad_path;
    for (const auto& operation : patch.array_range())
    {
        unwinder.state =jsoncons::jsonpatch::detail::state_type::begin;

        if (operation.count(detail::op_literal<char_type>()) != 1 || operation.count(detail::path_literal<char_type>()) != 1)
        {
            patch_ec = jsonpatch_errc::invalid_patch;
            unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
        }
        else
        {
            string_view_type op = operation.at(detail::op_literal<char_type>()).as_string_view();
            string_view_type path = operation.at(detail::path_literal<char_type>()).as_string_view();

            if (op ==jsoncons::jsonpatch::detail::test_literal<char_type>())
            {
                std::error_code ec;
                Json val = jsonpointer::get(target,path,ec);
                if (ec)
                {
                    patch_ec = jsonpatch_errc::test_failed;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else if (operation.count(detail::value_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else if (val != operation.at(detail::value_literal<char_type>()))
                {
                    patch_ec = jsonpatch_errc::test_failed;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
            }
            else if (op ==jsoncons::jsonpatch::detail::add_literal<char_type>())
            {
                if (operation.count(detail::value_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else
                {
                    std::error_code insert_ec;
                    Json val = operation.at(detail::value_literal<char_type>());
                    auto npath = jsonpatch::detail::normalized_path(target,path);
                    jsonpointer::add_if_absent(target,npath,val,insert_ec); // try insert without replace
                    if (insert_ec) // try a replace
                    {
                        std::error_code select_ec;
                        Json orig_val = jsonpointer::get(target,npath,select_ec);
                        if (select_ec) // shouldn't happen
                        {
                            patch_ec = jsonpatch_errc::add_failed;
                            unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                        }
                        else
                        {
                            std::error_code replace_ec;
                            jsonpointer::replace(target,npath,val,replace_ec);
                            if (replace_ec)
                            {
                                patch_ec = jsonpatch_errc::add_failed;
                                unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                            }
                            else
                            {
                                unwinder.stack.emplace_back(detail::op_type::replace,npath,orig_val);
                            }
                        }
                    }
                    else // insert without replace succeeded
                    {
                        unwinder.stack.emplace_back(detail::op_type::remove,npath,Json::null());
                    }
                }
            }
            else if (op ==jsoncons::jsonpatch::detail::remove_literal<char_type>())
            {
                std::error_code ec;
                Json val = jsonpointer::get(target,path,ec);
                if (ec)
                {
                    patch_ec = jsonpatch_errc::remove_failed;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else
                {
                    jsonpointer::remove(target,path,ec);
                    if (ec)
                    {
                        patch_ec = jsonpatch_errc::remove_failed;
                        unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                    }
                    else
                    {
                        unwinder.stack.emplace_back(detail::op_type::add,string_type(path),val);
                    }
                }
            }
            else if (op ==jsoncons::jsonpatch::detail::replace_literal<char_type>())
            {
                std::error_code ec;
                Json val = jsonpointer::get(target,path,ec);
                if (ec)
                {
                    patch_ec = jsonpatch_errc::replace_failed;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else if (operation.count(detail::value_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else
                {
                    jsonpointer::replace(target,path,operation.at(detail::value_literal<char_type>()),ec);
                    if (ec)
                    {
                        patch_ec = jsonpatch_errc::replace_failed;
                        unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                    }
                    else
                    {
                        unwinder.stack.emplace_back(detail::op_type::replace,string_type(path),val);
                    }
                }
            }
            else if (op ==jsoncons::jsonpatch::detail::move_literal<char_type>())
            {
                if (operation.count(detail::from_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else
                {
                    string_view_type from = operation.at(detail::from_literal<char_type>()).as_string_view();
                    std::error_code ec;
                    Json val = jsonpointer::get(target,from,ec);
                    if (ec)
                    {
                        patch_ec = jsonpatch_errc::move_failed;
                        unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                    }
                    else 
                    {
                        jsonpointer::remove(target,from,ec);
                        if (ec)
                        {
                            patch_ec = jsonpatch_errc::move_failed;
                            unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                        }
                        else
                        {
                            unwinder.stack.emplace_back(detail::op_type::add,string_type(from),val);
                            // add
                            std::error_code insert_ec;
                            auto npath = jsonpatch::detail::normalized_path(target,path);
                            jsonpointer::add_if_absent(target,npath,val,insert_ec); // try insert without replace
                            if (insert_ec) // try a replace
                            {
                                std::error_code select_ec;
                                Json orig_val = jsonpointer::get(target,npath,select_ec);
                                if (select_ec) // shouldn't happen
                                {
                                    patch_ec = jsonpatch_errc::copy_failed;
                                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                                }
                                else
                                {
                                    std::error_code replace_ec;
                                    jsonpointer::replace(target, npath, val, replace_ec);
                                    if (replace_ec)
                                    {
                                        patch_ec = jsonpatch_errc::copy_failed;
                                        unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;

                                    }
                                    else
                                    {
                                        unwinder.stack.emplace_back(jsoncons::jsonpatch::detail::op_type::replace,npath,orig_val);
                                    }
                                    
                                }
                            }
                            else
                            {
                                unwinder.stack.emplace_back(detail::op_type::remove,npath,Json::null());
                            }
                        }           
                    }
                }
            }
            else if (op ==jsoncons::jsonpatch::detail::copy_literal<char_type>())
            {
                if (operation.count(detail::from_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                }
                else
                {
                    std::error_code ec;
                    string_view_type from = operation.at(detail::from_literal<char_type>()).as_string_view();
                    Json val = jsonpointer::get(target,from,ec);
                    if (ec)
                    {
                        patch_ec = jsonpatch_errc::copy_failed;
                        unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                    }
                    else
                    {
                        // add
                        auto npath = jsonpatch::detail::normalized_path(target,path);
                        std::error_code insert_ec;
                        jsonpointer::add_if_absent(target,npath,val,insert_ec); // try insert without replace
                        if (insert_ec) // Failed, try a replace
                        {
                            std::error_code select_ec;
                            Json orig_val = jsonpointer::get(target,npath, select_ec);
                            if (select_ec) // shouldn't happen
                            {
                                patch_ec = jsonpatch_errc::copy_failed;
                                unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                            }
                            else 
                            {
                                std::error_code replace_ec;
                                jsonpointer::replace(target, npath, val,replace_ec);
                                if (replace_ec)
                                {
                                    patch_ec = jsonpatch_errc::copy_failed;
                                    unwinder.state =jsoncons::jsonpatch::detail::state_type::abort;
                                }
                                else
                                {
                                    unwinder.stack.emplace_back(jsoncons::jsonpatch::detail::op_type::replace,npath,orig_val);
                                }
                            }
                        }
                        else
                        {
                            unwinder.stack.emplace_back(detail::op_type::remove,npath,Json::null());
                        }
                    }
                }
            }
            if (unwinder.state !=jsoncons::jsonpatch::detail::state_type::begin)
            {
                bad_path = string_type(path);
            }
        }
        if (unwinder.state !=jsoncons::jsonpatch::detail::state_type::begin)
        {
            break;
        }
    }
    if (unwinder.state ==jsoncons::jsonpatch::detail::state_type::begin)
    {
        unwinder.state =jsoncons::jsonpatch::detail::state_type::commit;
    }
}

template <class Json>
Json from_diff(const Json& source, const Json& target)
{
    std::basic_string<typename Json::char_type> path;
    return jsoncons::jsonpatch::detail::from_diff(source, target, path);
}

template <class Json>
void apply_patch(Json& target, const Json& patch)
{
    std::error_code ec;
    apply_patch(target, patch, ec);
    if (ec)
    {
        JSONCONS_THROW(jsonpatch_error(ec));
    }
}

}}

#endif
