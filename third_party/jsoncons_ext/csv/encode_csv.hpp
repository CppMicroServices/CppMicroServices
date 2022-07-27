/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_ENCODE_CSV_HPP
#define JSONCONS_CSV_ENCODE_CSV_HPP

#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_encoder.hpp>

namespace jsoncons { 
namespace csv {

    template <class T,class Container>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_char_container<Container>::value>::type 
    encode_csv(const T& j, Container& s, const basic_csv_encode_options<typename Container::value_type>& options = basic_csv_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;
        basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>> encoder(s,options);
        j.dump(encoder);
    }

    template <class T,class Container>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_char_container<Container>::value>::type 
    encode_csv(const T& val, Container& s, const basic_csv_encode_options<typename Container::value_type>& options = basic_csv_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;
        basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>> encoder(s,options);
        std::error_code ec;
        encode_traits<T,char_type>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template <class T, class CharT>
    typename std::enable_if<type_traits::is_basic_json<T>::value,void>::type 
    encode_csv(const T& j, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
    {
        using char_type = CharT;
        basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>> encoder(os,options);
        j.dump(encoder);
    }

    template <class T, class CharT>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,void>::type 
    encode_csv(const T& val, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
    {
        using char_type = CharT;
        basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>> encoder(os,options);
        std::error_code ec;
        encode_traits<T,CharT>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    // with temp_allocator_arg_t

    template <class T, class Container, class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_char_container<Container>::value>::type 
    encode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               const T& j, Container& s, const basic_csv_encode_options<typename Container::value_type>& options = basic_csv_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;
        basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>,TempAllocator> encoder(s, options, temp_alloc);
        j.dump(encoder);
    }

    template <class T, class Container, class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value &&
                            type_traits::is_back_insertable_char_container<Container>::value>::type 
    encode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               const T& val, Container& s, const basic_csv_encode_options<typename Container::value_type>& options = basic_csv_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;
        basic_csv_encoder<char_type,jsoncons::string_sink<std::basic_string<char_type>>,TempAllocator> encoder(s, options, temp_alloc);
        std::error_code ec;
        encode_traits<T,char_type>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    template <class T, class CharT, class TempAllocator>
    typename std::enable_if<type_traits::is_basic_json<T>::value,void>::type 
    encode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               const T& j, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
    {
        using char_type = CharT;
        basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>,TempAllocator> encoder(os, options, temp_alloc);
        j.dump(encoder);
    }

    template <class T, class CharT, class TempAllocator>
    typename std::enable_if<!type_traits::is_basic_json<T>::value,void>::type 
    encode_csv(temp_allocator_arg_t, const TempAllocator& temp_alloc,
               const T& val, std::basic_ostream<CharT>& os, const basic_csv_encode_options<CharT>& options = basic_csv_encode_options<CharT>())
    {
        using char_type = CharT;
        basic_csv_encoder<char_type,jsoncons::stream_sink<char_type>,TempAllocator> encoder(os, options, temp_alloc);
        std::error_code ec;
        encode_traits<T,CharT>::encode(val, encoder, json(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

} // namespace csv 
} // namespace jsoncons

#endif
