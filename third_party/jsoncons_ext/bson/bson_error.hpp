/// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_ERROR_HPP
#define JSONCONS_BSON_BSON_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { namespace bson {

enum class bson_errc
{
    success = 0,
    unexpected_eof = 1,
    source_error,
    invalid_utf8_text_string,
    max_nesting_depth_exceeded,
    string_length_is_non_positive,
    length_is_negative,
    number_too_large,
    datetime_too_small,
    datetime_too_large,
    expected_bson_document,
    unknown_type
};

class bson_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "jsoncons/bson";
    }
    std::string message(int ev) const override
    {
        switch (static_cast<bson_errc>(ev))
        {
            case bson_errc::unexpected_eof:
                return "Unexpected end of file";
            case bson_errc::source_error:
                return "Source error";
            case bson_errc::invalid_utf8_text_string:
                return "Illegal UTF-8 encoding in text string";
            case bson_errc::max_nesting_depth_exceeded:
                return "Data item nesting exceeds limit in options";
            case bson_errc::string_length_is_non_positive:
                return "Request for the length of a string returned a non-positive result";
            case bson_errc::length_is_negative:
                return "Request for the length of a binary returned a negative result";
            case bson_errc::unknown_type:
                return "An unknown type was found in the stream";
            case bson_errc::number_too_large:
                return "Number too large";
            case bson_errc::datetime_too_large:
                return "datetime too large";
            case bson_errc::datetime_too_small:
                return "datetime too small";
            case bson_errc::expected_bson_document:
                return "Expected BSON document";
            default:
                return "Unknown BSON parser error";
        }
    }
};

inline
const std::error_category& bson_error_category()
{
  static bson_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(bson_errc result)
{
    return std::error_code(static_cast<int>(result),bson_error_category());
}


}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::bson::bson_errc> : public true_type
    {
    };
}

#endif
