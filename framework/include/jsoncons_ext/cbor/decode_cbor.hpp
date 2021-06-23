// Copyright 2017 Daniel Parkerstd
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_DECODE_CBOR_HPP
#define JSONCONS_CBOR_DECODE_CBOR_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/decode_traits.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/cbor/cbor_reader.hpp>
#include <jsoncons_ext/cbor/cbor_cursor.hpp>

namespace jsoncons { 
namespace cbor {

    template<class T, class Source>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_byte_sequence<Source>::value,T>::type 
    decode_cbor(const Source& v, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_cbor_reader<jsoncons::bytes_source> reader(v, adaptor, options);
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
    decode_cbor(const Source& v, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        basic_cbor_cursor<bytes_source> cursor(v, options);
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
    decode_cbor(std::istream& is, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        cbor_stream_reader reader(is, adaptor, options);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }
     
    template<class T>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,T>::type 
    decode_cbor(std::istream& is, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        basic_cbor_cursor<binary_stream_source> cursor(is, options);
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
    decode_cbor(InputIt first, InputIt last,
                const cbor_decode_options& options = cbor_decode_options())
    {
        jsoncons::json_decoder<T> decoder;
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_cbor_reader<binary_iterator_source<InputIt>> reader(binary_iterator_source<InputIt>(first, last), adaptor, options);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T, class InputIt>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,T>::type 
    decode_cbor(InputIt first, InputIt last,
                const cbor_decode_options& options = cbor_decode_options())
    {
        basic_cbor_cursor<binary_iterator_source<InputIt>> cursor(binary_iterator_source<InputIt>(first, last), options);
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
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const Source& v, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        json_decoder<T,TempAllocator> decoder(temp_alloc);
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_cbor_reader<jsoncons::bytes_source,TempAllocator> reader(v, adaptor, options, temp_alloc);
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
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const Source& v, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        basic_cbor_cursor<bytes_source,TempAllocator> cursor(v, options, temp_alloc);
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
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                std::istream& is, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        json_decoder<T,TempAllocator> decoder(temp_alloc);
        auto adaptor = make_json_visitor_adaptor<json_visitor>(decoder);
        basic_cbor_reader<jsoncons::binary_stream_source,TempAllocator> reader(is, adaptor, options, temp_alloc);
        reader.read();
        if (!decoder.is_valid())
        {
            JSONCONS_THROW(ser_error(conv_errc::conversion_failed, reader.line(), reader.column()));
        }
        return decoder.get_result();
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,T>::type 
    decode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                std::istream& is, 
                const cbor_decode_options& options = cbor_decode_options())
    {
        basic_cbor_cursor<binary_stream_source,TempAllocator> cursor(is, options, temp_alloc);
        json_decoder<basic_json<char,sorted_policy,TempAllocator>,TempAllocator> decoder(temp_alloc, temp_alloc);

        std::error_code ec;
        T val = decode_traits<T,char>::decode(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

} // namespace cbor
} // namespace jsoncons

#endif
