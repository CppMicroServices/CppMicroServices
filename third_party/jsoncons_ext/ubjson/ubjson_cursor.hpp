// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_CURSOR_HPP
#define JSONCONS_UBJSON_UBJSON_CURSOR_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <istream> // std::basic_istream
#include <jsoncons/byte_string.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/staj_cursor.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons_ext/ubjson/ubjson_parser.hpp>

namespace jsoncons { 
namespace ubjson {

template<class Src=jsoncons::binary_stream_source,class Allocator=std::allocator<char>>
class basic_ubjson_cursor : public basic_staj_cursor<char>, private virtual ser_context
{
public:
    using source_type = Src;
    using char_type = char;
    using allocator_type = Allocator;
private:
    basic_ubjson_parser<Src,Allocator> parser_;
    basic_staj_visitor<char_type> cursor_visitor_;
    bool eof_;

    // Noncopyable and nonmoveable
    basic_ubjson_cursor(const basic_ubjson_cursor&) = delete;
    basic_ubjson_cursor& operator=(const basic_ubjson_cursor&) = delete;

public:
    using string_view_type = string_view;

    template <class Source>
    basic_ubjson_cursor(Source&& source,
                      const ubjson_decode_options& options = ubjson_decode_options(),
                      const Allocator& alloc = Allocator())
       : parser_(std::forward<Source>(source), options, alloc), 
         cursor_visitor_(accept_all), 
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    // Constructors that set parse error codes

    template <class Source>
    basic_ubjson_cursor(Source&& source, 
                        std::error_code& ec)
       : basic_ubjson_cursor(std::allocator_arg, Allocator(),
                             std::forward<Source>(source), 
                             ubjson_decode_options(), 
                             ec)
    {
    }

    template <class Source>
    basic_ubjson_cursor(Source&& source, 
                        const ubjson_decode_options& options,
                        std::error_code& ec)
       : basic_ubjson_cursor(std::allocator_arg, Allocator(),
                             std::forward<Source>(source), 
                             options, 
                             ec)
    {
    }

    template <class Source>
    basic_ubjson_cursor(std::allocator_arg_t, const Allocator& alloc, 
                        Source&& source,
                        const ubjson_decode_options& options,
                        std::error_code& ec)
       : parser_(std::forward<Source>(source), options, alloc), 
         cursor_visitor_(accept_all),
         eof_(false)
    {
        if (!done())
        {
            next(ec);
        }
    }

    bool done() const override
    {
        return parser_.done();
    }

    const staj_event& current() const override
    {
        return cursor_visitor_.event();
    }

    void read_to(basic_json_visitor<char_type>& visitor) override
    {
        std::error_code ec;
        read_to(visitor, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read_to(basic_json_visitor<char_type>& visitor,
                std::error_code& ec) override
    {
        if (staj_to_saj_event(cursor_visitor_.event(), visitor, *this, ec))
        {
            read_next(visitor, ec);
        }
    }

    void next() override
    {
        std::error_code ec;
        next(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void next(std::error_code& ec) override
    {
        read_next(ec);
    }

    const ser_context& context() const override
    {
        return *this;
    }

    bool eof() const
    {
        return eof_;
    }

    std::size_t line() const override
    {
        return parser_.line();
    }

    std::size_t column() const override
    {
        return parser_.column();
    }

    friend
    staj_filter_view operator|(basic_ubjson_cursor& cursor, 
                               std::function<bool(const staj_event&, const ser_context&)> pred)
    {
        return staj_filter_view(cursor, pred);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_ubjson_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter,
                      const ubjson_decode_options& options = ubjson_decode_options(),
                      const Allocator& alloc = Allocator())
       : parser_(std::forward<Source>(source), options, alloc), 
         cursor_visitor_(filter), 
         eof_(false)
    {
        if (!done())
        {
            next();
        }
    }

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_ubjson_cursor(Source&& source, 
                        std::function<bool(const staj_event&, const ser_context&)> filter,
                        std::error_code& ec)
       : basic_ubjson_cursor(std::allocator_arg, Allocator(),
                             std::forward<Source>(source), filter, ec)
    {
    }

    template <class Source>
    JSONCONS_DEPRECATED_MSG("Instead, use pipe syntax for filter")
    basic_ubjson_cursor(std::allocator_arg_t, const Allocator& alloc, 
                        Source&& source,
                        std::function<bool(const staj_event&, const ser_context&)> filter,
                        std::error_code& ec)
       : parser_(std::forward<Source>(source), alloc), 
         cursor_visitor_(filter),
         eof_(false)
    {
        if (!done())
        {
            next(ec);
        }
    }

    JSONCONS_DEPRECATED_MSG("Instead, use read_to(basic_json_visitor<char_type>&)")
    void read(basic_json_visitor<char_type>& visitor)
    {
        read_to(visitor);
    }

    JSONCONS_DEPRECATED_MSG("Instead, use read_to(basic_json_visitor<char_type>&, std::error_code&)")
    void read(basic_json_visitor<char_type>& visitor,
                 std::error_code& ec) 
    {
        read_to(visitor, ec);
    }
#endif
private:
    static bool accept_all(const staj_event&, const ser_context&) 
    {
        return true;
    }

    void read_next(std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.stopped())
        {
            parser_.parse(cursor_visitor_, ec);
            if (ec) return;
        }
    }

    void read_next(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        parser_.restart();
        while (!parser_.stopped())
        {
            parser_.parse(visitor, ec);
            if (ec) return;
        }
    }
};

using ubjson_stream_cursor = basic_ubjson_cursor<jsoncons::binary_stream_source>;
using ubjson_bytes_cursor = basic_ubjson_cursor<jsoncons::bytes_source>;

} // namespace ubjson
} // namespace jsoncons

#endif

