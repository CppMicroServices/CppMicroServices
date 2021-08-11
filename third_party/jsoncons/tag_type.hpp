// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TAG_TYPE_HPP
#define JSONCONS_TAG_TYPE_HPP

#include <ostream>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons {

struct null_type
{
    explicit null_type() = default; 
};

struct temp_allocator_arg_t
{
    explicit temp_allocator_arg_t() = default; 
};

constexpr temp_allocator_arg_t temp_allocator_arg{};

struct result_allocator_arg_t
{
    explicit result_allocator_arg_t() = default; 
};

constexpr result_allocator_arg_t result_allocator_arg{};

struct half_arg_t
{
    explicit half_arg_t() = default; 
};

constexpr half_arg_t half_arg{};

struct json_array_arg_t
{
    explicit json_array_arg_t() = default; 
};

constexpr json_array_arg_t json_array_arg{};

struct json_object_arg_t
{
    explicit json_object_arg_t() = default; 
};

constexpr json_object_arg_t json_object_arg{};

struct byte_string_arg_t
{
    explicit byte_string_arg_t() = default; 
};

constexpr byte_string_arg_t byte_string_arg{};

struct json_const_pointer_arg_t
{
    explicit json_const_pointer_arg_t() = default; 
};

constexpr json_const_pointer_arg_t json_const_pointer_arg{};
 
enum class semantic_tag : uint8_t 
{
    none = 0,
    undefined = 0x01,
    datetime = 0x02,
    epoch_second = 0x03,
    epoch_milli = 0x04,
    epoch_nano = 0x05,
    bigint = 0x06,
    bigdec = 0x07,
    bigfloat = 0x08,
    base16 = 0x09,
    base64 = 0x1a,
    base64url = 0x1b,
    uri = 0x0c,
    clamped = 0x0d,
    multi_dim_row_major = 0x0e,
    multi_dim_column_major = 0x0f,
    ext = 0x10
#if !defined(JSONCONS_NO_DEPRECATED)
    , big_integer = bigint
    , big_decimal = bigdec
    , big_float = bigfloat
    , date_time = datetime
    , timestamp = epoch_second
#endif
};

template <class CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, semantic_tag tag)
{
    JSONCONS_CSTRING(CharT,na_name,'n','/','a')
    JSONCONS_CSTRING(CharT,undefined_name,'u','n','d','e','f','i','n','e','d')
    JSONCONS_CSTRING(CharT,datetime_name,'d','a','t','e','t','i','m','e')
    JSONCONS_CSTRING(CharT,epoch_second_name,'e','p','o','c','h','-','s','e','c','o','n','d')
    JSONCONS_CSTRING(CharT,epoch_milli_name,'e','p','o','c','h','-','m','i','l','l','i')
    JSONCONS_CSTRING(CharT,epoch_nano_name,'e','p','o','c','h','-','n','a','n','o')
    JSONCONS_CSTRING(CharT,bigint_name,'b','i','g','i','n','t')
    JSONCONS_CSTRING(CharT,bigdec_name,'b','i','g','d','e','c')
    JSONCONS_CSTRING(CharT,bigfloat_name,'b','i','g','f','l','o','a','t')
    JSONCONS_CSTRING(CharT,base16_name,'b','a','s','e','l','6')
    JSONCONS_CSTRING(CharT,base64_name,'b','a','s','e','6','4')
    JSONCONS_CSTRING(CharT,base64url_name,'b','a','s','e','6','4','u','r','l')
    JSONCONS_CSTRING(CharT,uri_name,'u','r','i')
    JSONCONS_CSTRING(CharT,clamped_name,'c','l','a','m','p','e','d')
    JSONCONS_CSTRING(CharT,multi_dim_row_major_name,'m','u','l','t','i','-','d','i','m','-','r','o','w','-','m','a','j','o','r')
    JSONCONS_CSTRING(CharT,multi_dim_column_major_name,'m','u','l','t','i','-','d','i','m','-','c','o','l','u','m','n','-','m','a','j','o','r')
    JSONCONS_CSTRING(CharT,ext_name,'e','x','t')

    switch (tag)
    {
        case semantic_tag::none:
        {
            os << na_name;
            break;
        }
        case semantic_tag::undefined:
        {
            os << undefined_name;
            break;
        }
        case semantic_tag::datetime:
        {
            os << datetime_name;
            break;
        }
        case semantic_tag::epoch_second:
        {
            os << epoch_second_name;
            break;
        }
        case semantic_tag::epoch_milli:
        {
            os << epoch_milli_name;
            break;
        }
        case semantic_tag::epoch_nano:
        {
            os << epoch_nano_name;
            break;
        }
        case semantic_tag::bigint:
        {
            os << bigint_name;
            break;
        }
        case semantic_tag::bigdec:
        {
            os << bigdec_name;
            break;
        }
        case semantic_tag::bigfloat:
        {
            os << bigfloat_name;
            break;
        }
        case semantic_tag::base16:
        {
            os << base16_name;
            break;
        }
        case semantic_tag::base64:
        {
            os << base64_name;
            break;
        }
        case semantic_tag::base64url:
        {
            os << base64url_name;
            break;
        }
        case semantic_tag::uri:
        {
            os << uri_name;
            break;
        }
        case semantic_tag::clamped:
        {
            os << clamped_name;
            break;
        }
        case semantic_tag::multi_dim_row_major:
        {
            os << multi_dim_row_major_name;
            break;
        }
        case semantic_tag::multi_dim_column_major:
        {
            os << multi_dim_column_major_name;
            break;
        }
        case semantic_tag::ext:
        {
            os << ext_name;
            break;
        }
    }
    return os;
}

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use semantic_tag") typedef semantic_tag semantic_tag_type;
    JSONCONS_DEPRECATED_MSG("Instead, use byte_string_arg_t") typedef byte_string_arg_t bstr_arg_t;
    constexpr byte_string_arg_t bstr_arg{};
#endif

}

#endif
