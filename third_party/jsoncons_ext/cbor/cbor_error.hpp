/// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_ERROR_HPP
#define JSONCONS_CBOR_CBOR_ERROR_HPP

#include <system_error>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp> // jsoncons::ser_error

namespace jsoncons { namespace cbor {

enum class cbor_errc
{
    success = 0,
    unexpected_eof,
    source_error,
    invalid_decimal_fraction,
    invalid_bigfloat,
    invalid_utf8_text_string,
    too_many_items,
    too_few_items,
    number_too_large,
    stringref_too_large,
    max_nesting_depth_exceeded,
    unknown_type,
    illegal_chunked_string
};

class cbor_error_category_impl
   : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "jsoncons/cbor";
    }
    std::string message(int ev) const override
    {
        switch (static_cast<cbor_errc>(ev))
        {
            case cbor_errc::unexpected_eof:
                return "Unexpected end of file";
            case cbor_errc::source_error:
                return "Source error";
            case cbor_errc::invalid_decimal_fraction:
                return "Invalid decimal fraction";
            case cbor_errc::invalid_bigfloat:
                return "Invalid bigfloat";
            case cbor_errc::invalid_utf8_text_string:
                return "Illegal UTF-8 encoding in text string";
            case cbor_errc::too_many_items:
                return "Too many items were added to a CBOR map or array of known length";
            case cbor_errc::too_few_items:
                return "Too few items were added to a CBOR map or array of known length";
            case cbor_errc::number_too_large:
                return "Number exceeds implementation limits";
            case cbor_errc::stringref_too_large:
                return "stringref exceeds stringref map size";
            case cbor_errc::max_nesting_depth_exceeded:
                return "Data item nesting exceeds limit in options";
            case cbor_errc::unknown_type:
                return "An unknown type was found in the stream";
            case cbor_errc::illegal_chunked_string:
                return "An illegal type was found while parsing an indefinite length string";
            default:
                return "Unknown CBOR parser error";
        }
    }
};

inline
const std::error_category& cbor_error_category()
{
  static cbor_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(cbor_errc e)
{
    return std::error_code(static_cast<int>(e),cbor_error_category());
}


#if !defined(JSONCONS_NO_DEPRECATED)

JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error cbor_error;
JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error cbor_decode_error;
JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error cbor_reader_errc;
#endif

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::cbor::cbor_errc> : public true_type
    {
    };
}

#endif
