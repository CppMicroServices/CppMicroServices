// Copyright 2017 Daniel Parkerstd
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_ENCODE_CBOR_HPP
#define JSONCONS_CBOR_ENCODE_CBOR_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/encode_traits.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>

namespace jsoncons { 
namespace cbor {

    // to bytes 

    template<class T, class Container>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_cbor(const T& j, 
                Container& v, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        using char_type = typename T::char_type;
        basic_cbor_encoder<jsoncons::bytes_sink<Container>> encoder(v, options);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T, class Container>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_cbor(const T& val, Container& v, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        basic_cbor_encoder<jsoncons::bytes_sink<Container>> encoder(v, options);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    // stream

    template<class T>
    typename std::enable_if<type_traits::is_basic_json<T>::value,void>::type 
    encode_cbor(const T& j, 
                std::ostream& os, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        using char_type = typename T::char_type;
        cbor_stream_encoder encoder(os, options);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,void>::type 
    encode_cbor(const T& val, 
                std::ostream& os, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        cbor_stream_encoder encoder(os, options);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    // temp_allocator_arg

    // to bytes 

    template<class T, class Container, class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& j, 
                Container& v, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        using char_type = typename T::char_type;
        basic_cbor_encoder<bytes_sink<Container>,TempAllocator> encoder(v, options, temp_alloc);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T, class Container, class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val, 
                Container& v, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        basic_cbor_encoder<jsoncons::bytes_sink<Container>,TempAllocator> encoder(v, options, temp_alloc);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    // stream

    template<class T,class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& j, 
                std::ostream& os, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        using char_type = typename T::char_type;
        basic_cbor_encoder<binary_stream_sink,TempAllocator> encoder(os, options, temp_alloc);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,void>::type 
    encode_cbor(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val, 
                std::ostream& os, 
                const cbor_encode_options& options = cbor_encode_options())
    {
        std::error_code ec;
        encode_cbor(temp_allocator_arg, temp_alloc, val, os, options, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

} // namespace cbor
} // namespace jsoncons

#endif
