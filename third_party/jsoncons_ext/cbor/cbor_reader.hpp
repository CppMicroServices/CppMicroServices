// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_READER_HPP
#define JSONCONS_CBOR_CBOR_READER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>

namespace jsoncons { namespace cbor {

template <class Src,class Allocator=std::allocator<char>>
class basic_cbor_reader 
{
    using char_type = char;

    basic_cbor_parser<Src,Allocator> parser_;
    basic_json_visitor2_to_visitor_adaptor<char_type,Allocator> adaptor_;
    json_visitor2& visitor_;
public:
    template <class Source>
    basic_cbor_reader(Source&& source, 
                      json_visitor& visitor, 
                      const Allocator alloc)
       : basic_cbor_reader(std::forward<Source>(source),
                           visitor,
                           cbor_decode_options(),
                           alloc)
    {
    }

    template <class Source>
    basic_cbor_reader(Source&& source, 
                      json_visitor& visitor, 
                      const cbor_decode_options& options = cbor_decode_options(),
                      const Allocator alloc=Allocator())
       : parser_(std::forward<Source>(source), options, alloc),
         adaptor_(visitor, alloc), visitor_(adaptor_)
    {
    }
    template <class Source>
    basic_cbor_reader(Source&& source, 
                      json_visitor2& visitor, 
                      const Allocator alloc)
       : basic_cbor_reader(std::forward<Source>(source),
                           visitor,
                           cbor_decode_options(),
                           alloc)
    {
    }

    template <class Source>
    basic_cbor_reader(Source&& source, 
                      json_visitor2& visitor, 
                      const cbor_decode_options& options = cbor_decode_options(),
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

using cbor_stream_reader = basic_cbor_reader<jsoncons::binary_stream_source>;

using cbor_bytes_reader = basic_cbor_reader<jsoncons::bytes_source>;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use cbor_stream_reader") typedef cbor_stream_reader cbor_reader;
JSONCONS_DEPRECATED_MSG("Instead, use cbor_bytes_reader") typedef cbor_bytes_reader cbor_buffer_reader;
#endif

}}

#endif
