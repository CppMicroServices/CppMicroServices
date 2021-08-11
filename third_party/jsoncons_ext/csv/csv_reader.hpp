// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_READER_HPP
#define JSONCONS_CSV_CSV_READER_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <memory> // std::allocator
#include <utility> // std::move
#include <istream> // std::basic_istream
#include <jsoncons/source.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons_ext/csv/csv_error.hpp>
#include <jsoncons_ext/csv/csv_parser.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/buffer_reader.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Src=jsoncons::stream_source<CharT>,class Allocator=std::allocator<char>>
class basic_csv_reader 
{
    struct stack_item
    {
        stack_item() noexcept
           : array_begun_(false)
        {
        }

        bool array_begun_;
    };
    using char_type = CharT;
    using temp_allocator_type = Allocator;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT> char_allocator_type;

    basic_csv_reader(const basic_csv_reader&) = delete; 
    basic_csv_reader& operator = (const basic_csv_reader&) = delete; 

    basic_default_json_visitor<CharT> default_visitor_;

    basic_json_visitor<CharT>& visitor_;

    basic_csv_parser<CharT,Allocator> parser_;
    Src source_;
    buffer_reader<CharT,Allocator> buffer_reader_;

public:
    // Structural characters
    static constexpr size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of CSV text into a json object
    /*!
      \param is The input stream to read from
    */

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor, 
                     const Allocator& alloc = Allocator())

       : basic_csv_reader(std::forward<Source>(source), 
                          visitor, 
                          basic_csv_decode_options<CharT>(), 
                          default_csv_parsing(), 
                          alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     const basic_csv_decode_options<CharT>& options, 
                     const Allocator& alloc = Allocator())

        : basic_csv_reader(std::forward<Source>(source), 
                           visitor, 
                           options, 
                           default_csv_parsing(),
                           alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     std::function<bool(csv_errc,const ser_context&)> err_handler, 
                     const Allocator& alloc = Allocator())
        : basic_csv_reader(std::forward<Source>(source), 
                           visitor, 
                           basic_csv_decode_options<CharT>(), 
                           err_handler,
                           alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler, 
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Source>::value>::type* = 0)
       : visitor_(visitor),
         parser_(options, err_handler, alloc),
         source_(std::forward<Source>(source)),
         buffer_reader_(default_max_buffer_length,alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler, 
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Source>::value>::type* = 0)
       : visitor_(visitor),
         parser_(options, err_handler, alloc),
         buffer_reader_(0,alloc)
    {
        jsoncons::basic_string_view<CharT> sv(std::forward<Source>(source));
        auto r = unicode_traits::detect_encoding_from_bom(sv.data(), sv.size());
        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
        {
            JSONCONS_THROW(ser_error(json_errc::illegal_unicode_character,parser_.line(),parser_.column()));
        }
        std::size_t offset = (r.ptr - sv.data());
        parser_.update(sv.data()+offset,sv.size()-offset);
    }

    ~basic_csv_reader() noexcept = default;

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read(std::error_code& ec)
    {
        read_internal(ec);
    }

    std::size_t line() const
    {
        return parser_.line();
    }

    std::size_t column() const
    {
        return parser_.column();
    }

    bool eof() const
    {
        return buffer_reader_.eof();
    }

    std::size_t buffer_length() const
    {
        return buffer_reader_.buffer_length();
    }

    void buffer_length(std::size_t size)
    {
        buffer_reader_.buffer_length(size);
    }

private:

    void read_internal(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = csv_errc::source_error;
            return;
        }   
        while (!parser_.finished())
        {
            if (parser_.source_exhausted())
            {
                buffer_reader_.read(source_, ec);
                if (ec) return;
                if (!buffer_reader_.eof())
                {
                    parser_.update(buffer_reader_.data(),buffer_reader_.length());
                }
            }
            parser_.parse_some(visitor_, ec);
            if (ec) return;
        }
    }
};

using csv_reader = basic_csv_reader<char>;
using wcsv_reader = basic_csv_reader<wchar_t>;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use csv_reader") typedef csv_reader csv_string_reader;
JSONCONS_DEPRECATED_MSG("Instead, use wcsv_reader") typedef wcsv_reader wcsv_string_reader;
#endif

}}

#endif
