// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_HPP
#define JSONCONS_JSON_TYPE_HPP

#include <ostream>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {

    enum class json_type : uint8_t 
    {
        null_value,
        bool_value,
        int64_value,
        uint64_value,
        half_value,
        double_value,
        string_value,
        byte_string_value,
        array_value,
        object_value
    };

    template <class CharT>
    std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, json_type type)
    {
        JSONCONS_CSTRING(CharT,null_value,'n','u','l','l')
        JSONCONS_CSTRING(CharT,bool_value,'b','o','o','l')
        JSONCONS_CSTRING(CharT,int64_value,'i','n','t','6','4')
        JSONCONS_CSTRING(CharT,uint64_value,'u','i','n','t','6','4')
        JSONCONS_CSTRING(CharT,half_value,'h','a','l','f')
        JSONCONS_CSTRING(CharT,double_value,'d','o','u','b','l','e')
        JSONCONS_CSTRING(CharT,string_value,'s','t','r','i','n','g')
        JSONCONS_CSTRING(CharT,byte_string_value,'b','y','t','e',' ','s','t','r','i','n','g')
        JSONCONS_CSTRING(CharT,array_value,'a','r','r','a','y')
        JSONCONS_CSTRING(CharT,object_value,'o','b','j','e','c','t')

        switch (type)
        {
            case json_type::null_value:
            {
                os << null_value;
                break;
            }
            case json_type::bool_value:
            {
                os << bool_value;
                break;
            }
            case json_type::int64_value:
            {
                os << int64_value;
                break;
            }
            case json_type::uint64_value:
            {
                os << uint64_value;
                break;
            }
            case json_type::half_value:
            {
                os << half_value;
                break;
            }
            case json_type::double_value:
            {
                os << double_value;
                break;
            }
            case json_type::string_value:
            {
                os << string_value;
                break;
            }
            case json_type::byte_string_value:
            {
                os << byte_string_value;
                break;
            }
            case json_type::array_value:
            {
                os << array_value;
                break;
            }
            case json_type::object_value:
            {
                os << object_value;
                break;
            }
        }
        return os;
    }

    enum class storage_kind : uint8_t 
    {
        null_value = 0x00,
        bool_value = 0x01,
        int64_value = 0x02,
        uint64_value = 0x03,
        half_value = 0x04,
        double_value = 0x05,
        short_string_value = 0x06,
        long_string_value = 0x07,
        byte_string_value = 0x08,
        array_value = 0x09,
        empty_object_value = 0x0a,
        object_value = 0x0b,
        json_const_pointer = 0x0c
    };

    template <class CharT>
    std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, storage_kind storage)
    {
        JSONCONS_CSTRING(CharT,null_value,'n','u','l','l')
        JSONCONS_CSTRING(CharT,bool_value,'b','o','o','l')
        JSONCONS_CSTRING(CharT,int64_value,'i','n','t','6','4')
        JSONCONS_CSTRING(CharT,uint64_value,'u','i','n','t','6','4')
        JSONCONS_CSTRING(CharT,half_value,'h','a','l','f')
        JSONCONS_CSTRING(CharT,double_value,'d','o','u','b','l','e')
        JSONCONS_CSTRING(CharT,short_string_value,'s','h','o','r','t',' ','s','t','r','i','n','g')
        JSONCONS_CSTRING(CharT,long_string_value,'l','o','n','g',' ','s','t','r','i','n','g')
        JSONCONS_CSTRING(CharT,byte_string_value,'b','y','t','e',' ','s','t','r','i','n','g')
        JSONCONS_CSTRING(CharT,array_value,'a','r','r','a','y')
        JSONCONS_CSTRING(CharT,empty_object_value,'e','m','p','t','y',' ','o','b','j','e','c','t')
        JSONCONS_CSTRING(CharT,object_value,'o','b','j','e','c','t')
        JSONCONS_CSTRING(CharT,json_const_pointer,'j','s','o','n',' ','c','o','n','s','t',' ','p','o','i','n','t','e','r')

        switch (storage)
        {
            case storage_kind::null_value:
            {
                os << null_value;
                break;
            }
            case storage_kind::bool_value:
            {
                os << bool_value;
                break;
            }
            case storage_kind::int64_value:
            {
                os << int64_value;
                break;
            }
            case storage_kind::uint64_value:
            {
                os << uint64_value;
                break;
            }
            case storage_kind::half_value:
            {
                os << half_value;
                break;
            }
            case storage_kind::double_value:
            {
                os << double_value;
                break;
            }
            case storage_kind::short_string_value:
            {
                os << short_string_value;
                break;
            }
            case storage_kind::long_string_value:
            {
                os << long_string_value;
                break;
            }
            case storage_kind::byte_string_value:
            {
                os << byte_string_value;
                break;
            }
            case storage_kind::array_value:
            {
                os << array_value;
                break;
            }
            case storage_kind::empty_object_value:
            {
                os << empty_object_value;
                break;
            }
            case storage_kind::object_value:
            {
                os << object_value;
                break;
            }
            case storage_kind::json_const_pointer:
            {
                os << json_const_pointer;
                break;
            }
        }
        return os;
    }

} // jsoncons

#endif
