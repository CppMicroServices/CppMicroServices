// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_READER_HPP
#define JSONCONS_UBJSON_UBJSON_READER_HPP

#include <string>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/ubjson/ubjson_detail.hpp>
#include <jsoncons_ext/ubjson/ubjson_error.hpp>
#include <jsoncons_ext/ubjson/ubjson_parser.hpp>

namespace jsoncons { namespace ubjson {

template <class Src,class Allocator=std::allocator<char>>
class basic_ubjson_reader
{
    basic_ubjson_parser<Src,Allocator> parser_;
    json_visitor& visitor_;
public:
    template <class Source>
    basic_ubjson_reader(Source&& source, 
                      json_visitor& visitor, 
                      const Allocator alloc)
       : basic_ubjson_reader(std::forward<Source>(source),
                           visitor,
                           ubjson_decode_options(),
                           alloc)
    {
    }

    template <class Source>
    basic_ubjson_reader(Source&& source, 
                      json_visitor& visitor, 
                      const ubjson_decode_options& options = ubjson_decode_options(),
                      const Allocator alloc=Allocator())
       : parser_(std::forward<Source>(source), options, alloc),
         visitor_(visitor)
    {
    }

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,line(),column()));
        }
    }

    void read(std::error_code& ec)
    {
        parser_.reset();
        parser_.parse(visitor_, ec);
        if (ec)
        {
            return;
        }
    }

    std::size_t line() const 
    {
        return parser_.line();
    }

    std::size_t column() const
    {
        return parser_.column();
    }
};

using ubjson_stream_reader = basic_ubjson_reader<jsoncons::binary_stream_source>;

using ubjson_bytes_reader = basic_ubjson_reader<jsoncons::bytes_source>;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use ubjson_stream_reader") typedef ubjson_stream_reader ubjson_reader;
JSONCONS_DEPRECATED_MSG("Instead, use ubjson_bytes_reader") typedef ubjson_bytes_reader ubjson_buffer_reader;
#endif

}}

#endif
