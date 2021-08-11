// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_DECODE_UBJSON_HPP
#define JSONCONS_UBJSON_DECODE_UBJSON_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/ubjson/ubjson_reader.hpp>
#include <jsoncons_ext/ubjson/ubjson_cursor.hpp>

namespace jsoncons { 
namespace ubjson {

    template<class T, class Source>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_byte_sequence<Source>::value,T>::type 
    decode_ubjson(const Source& v, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_ubjson_reader<jsoncons::bytes_source> reader(v, adaptor, options);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T, class Source>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_byte_sequence<Source>::value,T>::type 
    decode_ubjson(const Source& v, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        basic_ubjson_cursor<bytes_source> cursor(v, options);
        json_decoder<basic_json<char,sorted_policy>> decoder{};

        std::error_code ec;
        T val = decode_traits<T,char>::decode(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template<class T>
    typename std::enable_if<type_traits::is_basic_json<T>::value,T>::type 
    decode_ubjson(std::istream& is, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        ubjson_stream_reader reader(is, adaptor, options);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,T>::type 
    decode_ubjson(std::istream& is, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        basic_ubjson_cursor<binary_stream_source> cursor(is, options);
        json_decoder<basic_json<char,sorted_policy>> decoder{};

        std::error_code ec;
        T val = decode_traits<T,char>::decode(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template<class T, class InputIt>
    typename std::enable_if<type_traits::is_basic_json<T>::value,T>::type 
    decode_ubjson(InputIt first, InputIt last,
                const ubjson_decode_options& options = ubjson_decode_options())
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_ubjson_reader<binary_iterator_source<InputIt>> reader(binary_iterator_source<InputIt>(first, last), adaptor, options);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T, class InputIt>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,T>::type 
    decode_ubjson(InputIt first, InputIt last,
                const ubjson_decode_options& options = ubjson_decode_options())
    {
        basic_ubjson_cursor<binary_iterator_source<InputIt>> cursor(binary_iterator_source<InputIt>(first, last), options);
        json_decoder<basic_json<char,sorted_policy>> decoder{};

        std::error_code ec;
        T val = decode_traits<T,char>::decode(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    // With leading allocator parameter

    template<class T, class Source, class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_byte_sequence<Source>::value,T>::type 
    decode_ubjson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                  const Source& v, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        json_decoder<T,TempAllocator> decoder(temp_alloc);
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_ubjson_reader<jsoncons::bytes_source,TempAllocator> reader(v, adaptor, options, temp_alloc);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T, class Source, class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_byte_sequence<Source>::value,T>::type 
    decode_ubjson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                  const Source& v, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        basic_ubjson_cursor<bytes_source,TempAllocator> cursor(v, options, temp_alloc);
        json_decoder<basic_json<char,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = decode_traits<T,char>::decode(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template<class T,class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value,T>::type 
    decode_ubjson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                  std::istream& is, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        json_decoder<T,TempAllocator> decoder(temp_alloc);
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_ubjson_reader<jsoncons::binary_stream_source,TempAllocator> reader(is, adaptor, options, temp_alloc);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,T>::type 
    decode_ubjson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                  std::istream& is, 
                  const ubjson_decode_options& options = ubjson_decode_options())
    {
        basic_ubjson_cursor<binary_stream_source,TempAllocator> cursor(is, options, temp_alloc);
        json_decoder<basic_json<char,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = decode_traits<T,char>::decode(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

} // ubjson
} // jsoncons

#endif
