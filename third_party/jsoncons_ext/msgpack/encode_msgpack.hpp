// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_ENCODE_MSGPACK_HPP
#define JSONCONS_MSGPACK_ENCODE_MSGPACK_HPP

#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::enable_if
#include <istream> // std::basic_istream
#include <jsoncons/json.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/msgpack/msgpack_encoder.hpp>
#include <jsoncons_ext/msgpack/msgpack_reader.hpp>

namespace jsoncons { 
namespace msgpack {

    template<class T, class Container>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_msgpack(const T& j, 
                   Container& v, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        basic_msgpack_encoder<jsoncons::bytes_sink<Container>> encoder(v, options);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T, class Container>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_msgpack(const T& val, 
                   Container& v, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        basic_msgpack_encoder<jsoncons::bytes_sink<Container>> encoder(v, options);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template<class T>
    typename std::enable_if<type_traits::is_basic_json<T>::value,void>::type 
    encode_msgpack(const T& j, 
                   std::ostream& os, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        msgpack_stream_encoder encoder(os, options);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,void>::type 
    encode_msgpack(const T& val, 
                   std::ostream& os, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        msgpack_stream_encoder encoder(os, options);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    // with temp_allocator_arg_t

    template<class T, class Container, class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_msgpack(temp_allocator_arg_t, const TempAllocator& temp_alloc, const T& j, 
                   Container& v, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        basic_msgpack_encoder<jsoncons::bytes_sink<Container>,TempAllocator> encoder(v, options, temp_alloc);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T, class Container, class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_byte_container<Container>::value,void>::type 
    encode_msgpack(temp_allocator_arg_t, const TempAllocator& temp_alloc, 
                   const T& val, Container& v, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        basic_msgpack_encoder<jsoncons::bytes_sink<Container>,TempAllocator> encoder(v, options, temp_alloc);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template<class T,class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value,void>::type 
    encode_msgpack(temp_allocator_arg_t, const TempAllocator& temp_alloc, 
                   const T& j, 
                   std::ostream& os, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        using char_type = typename T::char_type;
        basic_msgpack_encoder<jsoncons::binary_stream_sink,TempAllocator> encoder(os, options, temp_alloc);
        auto adaptor = make_json_visitor_adaptor<basic_json_visitor<char_type>>(encoder);
        j.dump(adaptor);
    }

    template<class T,class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,void>::type 
    encode_msgpack(temp_allocator_arg_t, const TempAllocator& temp_alloc, 
                   const T& val, 
                   std::ostream& os, 
                   const msgpack_encode_options& options = msgpack_encode_options())
    {
        basic_msgpack_encoder<jsoncons::binary_stream_sink,TempAllocator> encoder(os, options, temp_alloc);
        std::error_code ec;
        encode_traits<T,char>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

} // msgpack
} // jsoncons

#endif
