// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPOINTER_HPP
#define JSONCONS_JSONPOINTER_JSONPOINTER_HPP

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <iterator>
#include <utility> // std::move
#include <system_error> // system_error
#include <type_traits> // std::enable_if, std::true_type
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer_error.hpp>
#include <jsoncons/detail/write_number.hpp>

namespace jsoncons { namespace jsonpointer {

    namespace detail {

    enum class pointer_state 
    {
        start,
        escaped,
        delim
    };

    } // namespace detail

    // json_pointer_iterator
    template <class InputIt>
    class json_pointer_iterator
    {
        using char_type = typename std::iterator_traits<InputIt>::value_type;
        using string_type = std::basic_string<char_type>;
        using base_iterator = InputIt;

        base_iterator path_ptr_;
        base_iterator end_input_;
        base_iterator p_;
        base_iterator q_;
        jsonpointer::detail::pointer_state state_;
        std::size_t column_;
        std::basic_string<char_type> buffer_;
    public:
        using value_type = string_type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::input_iterator_tag;

        json_pointer_iterator(base_iterator first, base_iterator last)
            : json_pointer_iterator(first, last, first)
        {
            std::error_code ec;
            increment(ec);
        }

        json_pointer_iterator(base_iterator first, base_iterator last, base_iterator current)
            : path_ptr_(first), end_input_(last), p_(current), q_(current), state_(jsonpointer::detail::pointer_state::start),
              column_(0)
        {
        }

        json_pointer_iterator(const json_pointer_iterator&) = default;

        json_pointer_iterator(json_pointer_iterator&&) = default;

        json_pointer_iterator& operator=(const json_pointer_iterator&) = default;

        json_pointer_iterator& operator=(json_pointer_iterator&&) = default;

        json_pointer_iterator& operator++()
        {
            std::error_code ec;
            increment(ec);
            if (ec)
            {
                JSONCONS_THROW(jsonpointer_error(ec));
            }
            return *this;
        }

        json_pointer_iterator& increment(std::error_code& ec)
        {
            q_ = p_;
            if (p_ != end_input_)
            {
                buffer_.clear();
                bool done = false;
                while (p_ != end_input_ && !done)
                {
                    switch (state_)
                    {
                        case jsonpointer::detail::pointer_state::start: 
                            switch (*p_)
                            {
                                case '/':
                                    state_ = jsonpointer::detail::pointer_state::delim;
                                    break;
                                default:
                                    ec = jsonpointer_errc::expected_slash;
                                    done = true;
                                    break;
                            };
                            break;
                        case jsonpointer::detail::pointer_state::delim: 
                            switch (*p_)
                            {
                                case '/':
                                    done = true;
                                    break;
                                case '~':
                                    state_ = jsonpointer::detail::pointer_state::escaped;
                                    break;
                                default:
                                    buffer_.push_back(*p_);
                                    break;
                            };
                            break;
                        case jsonpointer::detail::pointer_state::escaped: 
                            switch (*p_)
                            {
                                case '0':
                                    buffer_.push_back('~');
                                    state_ = jsonpointer::detail::pointer_state::delim;
                                    break;
                                case '1':
                                    buffer_.push_back('/');
                                    state_ = jsonpointer::detail::pointer_state::delim;
                                    break;
                                default:
                                    ec = jsonpointer_errc::expected_0_or_1;
                                    done = true;
                                    break;
                            };
                            break;
                        default:
                            JSONCONS_UNREACHABLE();
                            break;
                    }
                    ++p_;
                    ++column_;
                }
            }
            return *this;
        }

        json_pointer_iterator operator++(int) // postfix increment
        {
            json_pointer_iterator temp(*this);
            ++(*this);
            return temp;
        }

        reference operator*() const
        {
            return buffer_;
        }

        friend bool operator==(const json_pointer_iterator& it1, const json_pointer_iterator& it2)
        {
            return it1.q_ == it2.q_;
        }
        friend bool operator!=(const json_pointer_iterator& it1, const json_pointer_iterator& it2)
        {
            return !(it1 == it2);
        }

    private:
    };

    template <class CharT>
    std::basic_string<CharT> escape_string(const std::basic_string<CharT>& s)
    {
        std::basic_string<CharT> result;
        for (auto c : s)
        {
            switch (c)
            {
                case '~':
                    result.push_back('~');
                    result.push_back('0');
                    break;
                case '/':
                    result.push_back('~');
                    result.push_back('1');
                    break;
                default:
                    result.push_back(c);
                    break;
            }
        }
        return result;
    }

    // basic_json_pointer

    template <class CharT>
    class basic_json_pointer
    {
    public:
        std::basic_string<CharT> path_;
    public:
        // Member types
        using char_type = CharT;
        using string_type = std::basic_string<char_type>;
        using string_view_type = jsoncons::basic_string_view<char_type>;
        using const_iterator = json_pointer_iterator<typename string_type::const_iterator>;
        using iterator = const_iterator;

        // Constructors
        basic_json_pointer()
        {
        }
        explicit basic_json_pointer(const string_type& s)
            : path_(s)
        {
        }
        explicit basic_json_pointer(string_type&& s)
            : path_(std::move(s))
        {
        }
        explicit basic_json_pointer(const CharT* s)
            : path_(s)
        {
        }

        basic_json_pointer(const basic_json_pointer&) = default;

        basic_json_pointer(basic_json_pointer&&) = default;

        // operator=
        basic_json_pointer& operator=(const basic_json_pointer&) = default;

        basic_json_pointer& operator=(basic_json_pointer&&) = default;

        // Modifiers

        void clear()
        {
            path_.clear();
        }

        basic_json_pointer& operator/=(const string_type& s) 
        {
            path_.push_back('/');
            path_.append(escape_string(s));

            return *this;
        }

        template <class IntegerType>
        typename std::enable_if<type_traits::is_integer<IntegerType>::value, basic_json_pointer&>::type
        operator/=(IntegerType val)
        {
            string_type s;
            jsoncons::detail::from_integer(val, s);
            path_.push_back('/');
            path_.append(s);

            return *this;
        }

        basic_json_pointer& operator+=(const basic_json_pointer& p)
        {
            path_.append(p.path_);
            return *this;
        }

        // Accessors
        bool empty() const
        {
          return path_.empty();
        }

        const string_type& string() const
        {
            return path_;
        }

        operator string_view_type() const
        {
            return path_;
        }

        // Iterators
        iterator begin() const
        {
            return iterator(path_.begin(),path_.end());
        }
        iterator end() const
        {
            return iterator(path_.begin(), path_.end(), path_.end());
        }

        // Non-member functions
        friend basic_json_pointer<CharT> operator/(const basic_json_pointer<CharT>& lhs, const string_type& rhs)
        {
            basic_json_pointer<CharT> p(lhs);
            p /= rhs;
            return p;
        }

        friend basic_json_pointer<CharT> operator+( const basic_json_pointer<CharT>& lhs, const basic_json_pointer<CharT>& rhs )
        {
            basic_json_pointer<CharT> p(lhs);
            p += rhs;
            return p;
        }

        friend bool operator==( const basic_json_pointer& lhs, const basic_json_pointer& rhs )
        {
            return lhs.path_ == rhs.path_;
        }

        friend bool operator!=( const basic_json_pointer& lhs, const basic_json_pointer& rhs )
        {
            return lhs.path_ != rhs.path_;
        }

        friend std::basic_ostream<CharT>&
        operator<<( std::basic_ostream<CharT>& os, const basic_json_pointer<CharT>& p )
        {
            os << p.path_;
            return os;
        }
    };

    template <class CharT,class IntegerType>
    typename std::enable_if<type_traits::is_integer<IntegerType>::value, basic_json_pointer<CharT>>::type
    operator/(const basic_json_pointer<CharT>& lhs, IntegerType rhs)
    {
        basic_json_pointer<CharT> p(lhs);
        p /= rhs;
        return p;
    }

    using json_pointer = basic_json_pointer<char>;
    using wjson_pointer = basic_json_pointer<wchar_t>;

    #if !defined(JSONCONS_NO_DEPRECATED)
    template<class CharT>
    using basic_address = basic_json_pointer<CharT>;
    template<class CharT>
    using basic_json_ptr = basic_json_pointer<CharT>;
    JSONCONS_DEPRECATED_MSG("Instead, use json_pointer") typedef json_pointer address;
    JSONCONS_DEPRECATED_MSG("Instead, use json_pointer") typedef json_pointer json_ptr;
    JSONCONS_DEPRECATED_MSG("Instead, use wjson_pointer") typedef json_pointer wjson_ptr;
    #endif

    namespace detail {

    template <class Json>
    const Json* resolve(const Json* current, const typename Json::string_view_type& buffer, std::error_code& ec)
    {
        if (current->is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return current;
            }
            auto result = jsoncons::detail::to_integer_decimal<std::size_t>(buffer.data(), buffer.length());
            if (!result)
            {
                ec = jsonpointer_errc::invalid_index;
                return current;
            }
            std::size_t index = result.value();
            if (index >= current->size())
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return current;
            }
            current = std::addressof(current->at(index));
        }
        else if (current->is_object())
        {
            if (!current->contains(buffer))
            {
                ec = jsonpointer_errc::key_not_found;
                return current;
            }
            current = std::addressof(current->at(buffer));
        }
        else
        {
            ec = jsonpointer_errc::expected_object_or_array;
            return current;
        }
        return current;
    }

    template <class Json>
    Json* resolve(Json* current, const typename Json::string_view_type& buffer, bool create_if_missing, std::error_code& ec)
    {
        if (current->is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return current;
            }
            auto result = jsoncons::detail::to_integer_decimal<std::size_t>(buffer.data(), buffer.length());
            if (!result)
            {
                ec = jsonpointer_errc::invalid_index;
                return current;
            }
            std::size_t index = result.value();
            if (index >= current->size())
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return current;
            }
            current = std::addressof(current->at(index));
        }
        else if (current->is_object())
        {
            if (!current->contains(buffer))
            {
                if (create_if_missing)
                {
                    auto r = current->try_emplace(buffer, Json());
                    current = std::addressof(r.first->value());
                }
                else
                {
                    ec = jsonpointer_errc::key_not_found;
                    return current;
                }
            }
            else
            {
                current = std::addressof(current->at(buffer));
            }
        }
        else
        {
            ec = jsonpointer_errc::expected_object_or_array;
            return current;
        }
        return current;
    }

    } // namespace detail

    // get

    template<class Json>
    Json& get(Json& root, 
              const typename Json::string_view_type& location, 
              bool create_if_missing,
              std::error_code& ec)
    {
        if (location.empty())
        {
            return root;
        }

        Json* current = std::addressof(root);
        json_pointer_iterator<typename Json::string_view_type::iterator> it(location.begin(), location.end());
        json_pointer_iterator<typename Json::string_view_type::iterator> end(location.begin(), location.end(), location.end());
        while (it != end)
        {
            current = jsoncons::jsonpointer::detail::resolve(current, *it, create_if_missing, ec);
            if (ec)
                return *current;
            it.increment(ec);
            if (ec)
                return *current;
        }
        return *current;
    }

    template<class Json>
    const Json& get(const Json& root, const typename Json::string_view_type& location, std::error_code& ec)
    {
        if (location.empty())
        {
            return root;
        }

        const Json* current = std::addressof(root);
        json_pointer_iterator<typename Json::string_view_type::iterator> it(location.begin(), location.end());
        json_pointer_iterator<typename Json::string_view_type::iterator> end(location.begin(), location.end(), location.end());
        while (it != end)
        {
            current = jsoncons::jsonpointer::detail::resolve(current, *it, ec);
            if (ec)
                return *current;
            it.increment(ec);
            if (ec)
                return *current;
        }
        return *current;
    }

    template<class Json>
    Json& get(Json& root, 
              const typename Json::string_view_type& location, 
              std::error_code& ec)
    {
        return get(root, location, false, ec);
    }

    template<class Json>
    Json& get(Json& root, 
              const typename Json::string_view_type& location,
              bool create_if_missing = false)
    {
        std::error_code ec;
        Json& j = get(root, location, create_if_missing, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
        return j;
    }

    template<class Json>
    const Json& get(const Json& root, const typename Json::string_view_type& location)
    {
        std::error_code ec;
        const Json& j = get(root, location, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
        return j;
    }

    // contains

    template<class Json>
    bool contains(const Json& root, const typename Json::string_view_type& location)
    {
        std::error_code ec;
        get(root, location, ec);
        return !ec ? true : false;
    }

    // add

    template<class Json,class T>
    void add(Json& root, 
             const typename Json::string_view_type& location, 
             T&& value, 
             bool create_if_missing,
             std::error_code& ec)
    {
        Json* current = std::addressof(root);

        std::basic_string<typename Json::char_type> buffer;
        json_pointer_iterator<typename Json::string_view_type::iterator> it(location.begin(), location.end());
        json_pointer_iterator<typename Json::string_view_type::iterator> end(location.begin(), location.end(), location.end());
        while (it != end)
        {
            buffer = *it;
            it.increment(ec);
            if (ec)
                return;
            if (it != end)
            {
                current = jsoncons::jsonpointer::detail::resolve(current, buffer, create_if_missing, ec);
                if (ec)
                    return;
            }
        }
        if (current->is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                current->emplace_back(std::forward<T>(value));
                current = std::addressof(current->at(current->size()-1));
            }
            else
            {
                auto result = jsoncons::detail::to_integer_decimal<std::size_t>(buffer.data(), buffer.length());
                if (!result)
                {
                    ec = jsonpointer_errc::invalid_index;
                    return;
                }
                std::size_t index = result.value();
                if (index > current->size())
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                if (index == current->size())
                {
                    current->emplace_back(std::forward<T>(value));
                    current = std::addressof(current->at(current->size()-1));
                }
                else
                {
                    auto it2 = current->insert(current->array_range().begin()+index,std::forward<T>(value));
                    current = std::addressof(*it2);
                }
            }
        }
        else if (current->is_object())
        {
            auto r = current->insert_or_assign(buffer,std::forward<T>(value));
            current = std::addressof(r.first->value());
        }
        else
        {
            ec = jsonpointer_errc::expected_object_or_array;
            return;
        }
    }

    template<class Json,class T>
    void add(Json& root, 
             const typename Json::string_view_type& location, 
             T&& value, 
             std::error_code& ec)
    {
        add(root, location, std::forward<T>(value), false, ec);
    }

    template<class Json,class T>
    void add(Json& root, 
             const typename Json::string_view_type& location, 
             T&& value,
             bool create_if_missing = false)
    {
        std::error_code ec;
        add(root, location, std::forward<T>(value), create_if_missing, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    // add_if_absent

    template<class Json, class T>
    void add_if_absent(Json& root, 
                const typename Json::string_view_type& location, 
                T&& value, 
                bool create_if_missing,
                std::error_code& ec)
    {
        Json* current = std::addressof(root);

        std::basic_string<typename Json::char_type> buffer;
        json_pointer_iterator<typename Json::string_view_type::iterator> it(location.begin(), location.end());
        json_pointer_iterator<typename Json::string_view_type::iterator> end(location.begin(), location.end(), location.end());

        while (it != end)
        {
            buffer = *it;
            it.increment(ec);
            if (ec)
                return;
            if (it != end)
            {
                current = jsoncons::jsonpointer::detail::resolve(current, buffer, create_if_missing, ec);
                if (ec)
                    return;
            }
        }
        if (current->is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                current->emplace_back(std::forward<T>(value));
                current = std::addressof(current->at(current->size()-1));
            }
            else
            {
                auto result = jsoncons::detail::to_integer_decimal<std::size_t>(buffer.data(), buffer.length());
                if (!result)
                {
                    ec = jsonpointer_errc::invalid_index;
                    return;
                }
                std::size_t index = result.value();
                if (index > current->size())
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                if (index == current->size())
                {
                    current->emplace_back(std::forward<T>(value));
                    current = std::addressof(current->at(current->size()-1));
                }
                else
                {
                    auto it2 = current->insert(current->array_range().begin()+index,std::forward<T>(value));
                    current = std::addressof(*it2);
                }
            }
        }
        else if (current->is_object())
        {
            if (current->contains(buffer))
            {
                ec = jsonpointer_errc::key_already_exists;
                return;
            }
            else
            {
                auto r = current->try_emplace(buffer,std::forward<T>(value));
                current = std::addressof(r.first->value());
            }
        }
        else
        {
            ec = jsonpointer_errc::expected_object_or_array;
            return;
        }
    }

    template<class Json, class T>
    void add_if_absent(Json& root, 
                const typename Json::string_view_type& location, 
                T&& value, 
                std::error_code& ec)
    {
        add_if_absent(root, location, std::forward<T>(value), false, ec);
    }

    template<class Json, class T>
    void add_if_absent(Json& root, 
                const typename Json::string_view_type& location, 
                T&& value,
                bool create_if_missing = false)
    {
        std::error_code ec;
        add_if_absent(root, location, std::forward<T>(value), create_if_missing, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    // remove

    template<class Json>
    void remove(Json& root, const typename Json::string_view_type& location, std::error_code& ec)
    {
        Json* current = std::addressof(root);

        std::basic_string<typename Json::char_type> buffer;
        json_pointer_iterator<typename Json::string_view_type::iterator> it(location.begin(), location.end());
        json_pointer_iterator<typename Json::string_view_type::iterator> end(location.begin(), location.end(), location.end());

        while (it != end)
        {
            buffer = *it;
            it.increment(ec);
            if (ec)
                return;
            if (it != end)
            {
                current = jsoncons::jsonpointer::detail::resolve(current, buffer, false, ec);
                if (ec)
                    return;
            }
        }
        if (current->is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return;
            }
            else
            {
                auto result = jsoncons::detail::to_integer_decimal<std::size_t>(buffer.data(), buffer.length());
                if (!result)
                {
                    ec = jsonpointer_errc::invalid_index;
                    return;
                }
                std::size_t index = result.value();
                if (index >= current->size())
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                current->erase(current->array_range().begin()+index);
            }
        }
        else if (current->is_object())
        {
            if (!current->contains(buffer))
            {
                ec = jsonpointer_errc::key_not_found;
                return;
            }
            else
            {
                current->erase(buffer);
            }
        }
        else
        {
            ec = jsonpointer_errc::expected_object_or_array;
            return;
        }
    }

    template<class Json>
    void remove(Json& root, const typename Json::string_view_type& location)
    {
        std::error_code ec;
        remove(root, location, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    // replace

    template<class Json, class T>
    void replace(Json& root, 
                 const typename Json::string_view_type& location, 
                 T&& value, 
                 bool create_if_missing,
                 std::error_code& ec)
    {
        Json* current = std::addressof(root);

        std::basic_string<typename Json::char_type> buffer;
        json_pointer_iterator<typename Json::string_view_type::iterator> it(location.begin(), location.end());
        json_pointer_iterator<typename Json::string_view_type::iterator> end(location.begin(), location.end(), location.end());

        while (it != end)
        {
            buffer = *it;
            it.increment(ec);
            if (ec)
                return;
            if (it != end)
            {
                current = jsoncons::jsonpointer::detail::resolve(current, buffer, create_if_missing, ec);
                if (ec)
                    return;
            }
        }
        if (current->is_array())
        {
            if (buffer.size() == 1 && buffer[0] == '-')
            {
                ec = jsonpointer_errc::index_exceeds_array_size;
                return;
            }
            else
            {
                auto result = jsoncons::detail::to_integer_decimal<std::size_t>(buffer.data(), buffer.length());
                if (!result)
                {
                    ec = jsonpointer_errc::invalid_index;
                    return;
                }
                std::size_t index = result.value();
                if (index >= current->size())
                {
                    ec = jsonpointer_errc::index_exceeds_array_size;
                    return;
                }
                current->at(index) = std::forward<T>(value);
            }
        }
        else if (current->is_object())
        {
            if (!current->contains(buffer))
            {
                if (create_if_missing)
                {
                    current->try_emplace(buffer,std::forward<T>(value));
                }
                else
                {
                    ec = jsonpointer_errc::key_not_found;
                    return;
                }
            }
            else
            {
                auto r = current->insert_or_assign(buffer,std::forward<T>(value));
                current = std::addressof(r.first->value());
            }
        }
        else
        {
            ec = jsonpointer_errc::expected_object_or_array;
            return;
        }
    }

    template<class Json, class T>
    void replace(Json& root, 
                 const typename Json::string_view_type& location, 
                 T&& value, 
                 std::error_code& ec)
    {
        replace(root, location, std::forward<T>(value), false, ec);
    }

    template<class Json, class T>
    void replace(Json& root, 
                 const typename Json::string_view_type& location, 
                 T&& value, 
                 bool create_if_missing = false)
    {
        std::error_code ec;
        replace(root, location, std::forward<T>(value), create_if_missing, ec);
        if (ec)
        {
            JSONCONS_THROW(jsonpointer_error(ec));
        }
    }

    template <class String,class Result>
    typename std::enable_if<std::is_convertible<typename String::value_type,typename Result::value_type>::value>::type
    escape(const String& s, Result& result)
    {
        for (auto c : s)
        {
            if (c == '~')
            {
                result.push_back('~');
                result.push_back('0');
            }
            else if (c == '/')
            {
                result.push_back('~');
                result.push_back('1');
            }
            else
            {
                result.push_back(c);
            }
        }
    }

    template <class CharT>
    std::basic_string<CharT> escape(const jsoncons::basic_string_view<CharT>& s)
    {
        std::basic_string<CharT> result;

        for (auto c : s)
        {
            if (c == '~')
            {
                result.push_back('~');
                result.push_back('0');
            }
            else if (c == '/')
            {
                result.push_back('~');
                result.push_back('1');
            }
            else
            {
                result.push_back(c);
            }
        }
        return result;
    }

    // flatten

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
                    // Flatten empty array to null
                    //result.try_emplace(parent_key, null_type{});
                    //result[parent_key] = parent_value;
                    result.try_emplace(parent_key, parent_value);
                }
                else
                {
                    for (std::size_t i = 0; i < parent_value.size(); ++i)
                    {
                        string_type key(parent_key);
                        key.push_back('/');
                        jsoncons::detail::from_integer(i,key);
                        flatten_(key, parent_value.at(i), result);
                    }
                }
                break;
            }

            case json_type::object_value:
            {
                if (parent_value.empty())
                {
                    // Flatten empty object to null
                    //result.try_emplace(parent_key, null_type{});
                    //result[parent_key] = parent_value;
                    result.try_emplace(parent_key, parent_value);
                }
                else
                {
                    for (const auto& item : parent_value.object_range())
                    {
                        string_type key(parent_key);
                        key.push_back('/');
                        escape(jsoncons::basic_string_view<char_type>(item.key().data(),item.key().size()), key);
                        flatten_(key, item.value(), result);
                    }
                }
                break;
            }

            default:
            {
                // add primitive parent_value with its reference string
                //result[parent_key] = parent_value;
                result.try_emplace(parent_key, parent_value);
                break;
            }
        }
    }

    template<class Json>
    Json flatten(const Json& value)
    {
        Json result;
        std::basic_string<typename Json::char_type> parent_key;
        flatten_(parent_key, value, result);
        return result;
    }


    // unflatten

    enum class unflatten_options {none,assume_object = 1
    #if !defined(JSONCONS_NO_DEPRECATED)
,object = assume_object
#endif
};

    template<class Json>
    Json safe_unflatten (Json& value)
    {
        if (!value.is_object() || value.empty())
        {
            return value;
        }
        bool safe = true;
        std::size_t index = 0;
        for (const auto& item : value.object_range())
        {
            auto r = jsoncons::detail::to_integer_decimal<std::size_t>(item.key().data(),item.key().size());
            if (!r || (index++ != r.value()))
            {
                safe = false;
                break;
            }
        }

        if (safe)
        {
            Json j(json_array_arg);
            j.reserve(value.size());
            for (auto& item : value.object_range())
            {
                j.emplace_back(std::move(item.value()));
            }
            Json a(json_array_arg);
            for (auto& item : j.array_range())
            {
                a.emplace_back(safe_unflatten (item));
            }
            return a;
        }
        else
        {
            Json o(json_object_arg);
            for (auto& item : value.object_range())
            {
                o.try_emplace(item.key(), safe_unflatten (item.value()));
            }
            return o;
        }
    }

    template<class Json>
    jsoncons::optional<Json> try_unflatten_array(const Json& value)
    {
        using char_type = typename Json::char_type;

        if (JSONCONS_UNLIKELY(!value.is_object()))
        {
            JSONCONS_THROW(jsonpointer_error(jsonpointer_errc::argument_to_unflatten_invalid));
        }
        Json result;

        for (const auto& item: value.object_range())
        {
            Json* part = &result;
            basic_json_pointer<char_type> ptr(item.key());
            std::size_t index = 0;
            for (auto it = ptr.begin(); it != ptr.end(); )
            {
                auto s = *it;
                auto r = jsoncons::detail::to_integer_decimal<size_t>(s.data(), s.size());
                if (r && (index++ == r.value()))
                {
                    if (!part->is_array())
                    {
                        *part = Json(json_array_arg);
                    }
                    if (++it != ptr.end())
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
                else if (part->is_object())
                {
                    if (++it != ptr.end())
                    {
                        auto res = part->try_emplace(s,Json());
                        part = &(res.first->value());
                    }
                    else
                    {
                        auto res = part->try_emplace(s, item.value());
                        part = &(res.first->value());
                    }
                }
                else 
                {
                    return jsoncons::optional<Json>();
                }
            }
        }

        return result;
    }

    template<class Json>
    Json unflatten_to_object(const Json& value, unflatten_options options = unflatten_options::none)
    {
        using char_type = typename Json::char_type;

        if (JSONCONS_UNLIKELY(!value.is_object()))
        {
            JSONCONS_THROW(jsonpointer_error(jsonpointer_errc::argument_to_unflatten_invalid));
        }
        Json result;

        for (const auto& item: value.object_range())
        {
            Json* part = &result;
            basic_json_pointer<char_type> ptr(item.key());
            for (auto it = ptr.begin(); it != ptr.end(); )
            {
                auto s = *it;
                if (++it != ptr.end())
                {
                    auto res = part->try_emplace(s,Json());
                    part = &(res.first->value());
                }
                else
                {
                    auto res = part->try_emplace(s, item.value());
                    part = &(res.first->value());
                }
            }
        }

        return options == unflatten_options::none ? safe_unflatten (result) : result;
    }

    template<class Json>
    Json unflatten(const Json& value, unflatten_options options = unflatten_options::none)
    {
        if (options == unflatten_options::none)
        {
            jsoncons::optional<Json> j = try_unflatten_array(value);
            return j ? *j : unflatten_to_object(value,options);
        }
        else
        {
            return unflatten_to_object(value,options);
        }
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    template<class Json>
    JSONCONS_DEPRECATED_MSG("Instead, use add(Json&, const typename Json::string_view_type&, const Json&)")
    void insert_or_assign(Json& root, const typename Json::string_view_type& location, const Json& value)
    {
        add(root, location, value);
    }

    template<class Json>
    JSONCONS_DEPRECATED_MSG("Instead, use add(Json&, const typename Json::string_view_type&, const Json&, std::error_code&)")
    void insert_or_assign(Json& root, const typename Json::string_view_type& location, const Json& value, std::error_code& ec)
    {
        add(root, location, value, ec);
    }
    template<class Json, class T>
    void insert(Json& root, 
                const typename Json::string_view_type& location, 
                T&& value, 
                bool create_if_missing,
                std::error_code& ec)
    {
        add_if_absent(root,location,std::forward<T>(value),create_if_missing,ec);
    }

    template<class Json, class T>
    void insert(Json& root, 
                const typename Json::string_view_type& location, 
                T&& value, 
                std::error_code& ec)
    {
        add_if_absent(root, location, std::forward<T>(value), ec);
    }

    template<class Json, class T>
    void insert(Json& root, 
                const typename Json::string_view_type& location, 
                T&& value,
                bool create_if_missing = false)
    {
        add_if_absent(root, location, std::forward<T>(value), create_if_missing);
    }
#endif

} // namespace jsonpointer
} // namespace jsoncons

#endif
