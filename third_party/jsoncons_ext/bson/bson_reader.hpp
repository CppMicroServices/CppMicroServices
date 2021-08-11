// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_READER_HPP
#define JSONCONS_BSON_BSON_READER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/bson/bson_detail.hpp>
#include <jsoncons_ext/bson/bson_error.hpp>
#include <jsoncons_ext/bson/bson_parser.hpp>

namespace jsoncons { namespace bson {

template <class Src,class Allocator=std::allocator<char>>
class basic_bson_reader 
{
    basic_bson_parser<Src,Allocator> parser_;
    json_visitor& visitor_;
public:
    template <class Source>
    basic_bson_reader(Source&& source, 
                      json_visitor& visitor, 
                      const Allocator alloc)
       : basic_bson_reader(std::forward<Source>(source),
                           visitor,
                           bson_decode_options(),
                           alloc)
    {
    }

    template <class Source>
    basic_bson_reader(Source&& source, 
                      json_visitor& visitor, 
                      const bson_decode_options& options = bson_decode_options(),
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

using bson_stream_reader = basic_bson_reader<jsoncons::binary_stream_source>;
using bson_bytes_reader = basic_bson_reader<jsoncons::bytes_source>;

#if !defined(JSONCONS_NO_DEPRECATED) 
JSONCONS_DEPRECATED_MSG("Instead, use bson_stream_reader") typedef bson_stream_reader bson_reader;
JSONCONS_DEPRECATED_MSG("Instead, use bson_bytes_reader") typedef bson_bytes_reader bson_buffer_reader;
#endif

}}

#endif
