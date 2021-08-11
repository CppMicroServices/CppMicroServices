// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_READER_HPP
#define JSONCONS_JSON_READER_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <utility> // std::move
#include <jsoncons/source.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/buffer_reader.hpp>

namespace jsoncons {

// utf8_other_json_input_adapter

template <class CharT>
class json_utf8_to_other_visitor_adaptor : public json_visitor
{
public:
    using json_visitor::string_view_type;
private:
    basic_default_json_visitor<CharT> default_visitor_;
    basic_json_visitor<CharT>& other_visitor_;
    //std::function<bool(json_errc,const ser_context&)> err_handler_;

    // noncopyable and nonmoveable
    json_utf8_to_other_visitor_adaptor(const json_utf8_to_other_visitor_adaptor<CharT>&) = delete;
    json_utf8_to_other_visitor_adaptor<CharT>& operator=(const json_utf8_to_other_visitor_adaptor<CharT>&) = delete;

public:
    json_utf8_to_other_visitor_adaptor()
        : other_visitor_(default_visitor_)
    {
    }

    json_utf8_to_other_visitor_adaptor(basic_json_visitor<CharT>& other_visitor/*,
                                          std::function<bool(json_errc,const ser_context&)> err_handler*/)
        : other_visitor_(other_visitor)/*,
          err_handler_(err_handler)*/
    {
    }

private:

    void visit_flush() override
    {
        other_visitor_.flush();
    }

    bool visit_begin_object(semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return other_visitor_.begin_object(tag, context, ec);
    }

    bool visit_end_object(const ser_context& context, std::error_code& ec) override
    {
        return other_visitor_.end_object(context, ec);
    }

    bool visit_begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return other_visitor_.begin_array(tag, context, ec);
    }

    bool visit_end_array(const ser_context& context, std::error_code& ec) override
    {
        return other_visitor_.end_array(context, ec);
    }

    bool visit_key(const string_view_type& name, const ser_context& context, std::error_code& ec) override
    {
        std::basic_string<CharT> target;
        auto result = unicode_traits::convert(
            name.data(), name.size(), target, 
            unicode_traits::conv_flags::strict);
        if (result.ec != unicode_traits::conv_errc())
        {
            JSONCONS_THROW(ser_error(result.ec,context.line(),context.column()));
        }
        return other_visitor_.key(target, context, ec);
    }

    bool visit_string(const string_view_type& value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        std::basic_string<CharT> target;
        auto result = unicode_traits::convert(
            value.data(), value.size(), target, 
            unicode_traits::conv_flags::strict);
        if (result.ec != unicode_traits::conv_errc())
        {
            ec = result.ec;
            return false;
        }
        return other_visitor_.string_value(target, tag, context, ec);
    }

    bool visit_int64(int64_t value, 
                        semantic_tag tag, 
                        const ser_context& context,
                        std::error_code& ec) override
    {
        return other_visitor_.int64_value(value, tag, context, ec);
    }

    bool visit_uint64(uint64_t value, 
                         semantic_tag tag, 
                         const ser_context& context,
                         std::error_code& ec) override
    {
        return other_visitor_.uint64_value(value, tag, context, ec);
    }

    bool visit_half(uint16_t value, 
                       semantic_tag tag,
                       const ser_context& context,
                       std::error_code& ec) override
    {
        return other_visitor_.half_value(value, tag, context, ec);
    }

    bool visit_double(double value, 
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code& ec) override
    {
        return other_visitor_.double_value(value, tag, context, ec);
    }

    bool visit_bool(bool value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return other_visitor_.bool_value(value, tag, context, ec);
    }

    bool visit_null(semantic_tag tag, const ser_context& context, std::error_code& ec) override
    {
        return other_visitor_.null_value(tag, context, ec);
    }
};

template<class CharT,class Src=jsoncons::stream_source<CharT>,class Allocator=std::allocator<char>>
class basic_json_reader 
{
public:
    using char_type = CharT;
    using source_type = Src;
    using string_view_type = jsoncons::basic_string_view<CharT>;
private:
    typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<CharT> char_allocator_type;

    static constexpr size_t default_max_buffer_length = 16384;

    basic_default_json_visitor<CharT> default_visitor_;

    basic_json_visitor<CharT>& visitor_;

    basic_json_parser<CharT,Allocator> parser_;

    source_type source_;
    json_buffer_reader<CharT,Allocator> buffer_reader_;

    // Noncopyable and nonmoveable
    basic_json_reader(const basic_json_reader&) = delete;
    basic_json_reader& operator=(const basic_json_reader&) = delete;

public:
    template <class Source>
    explicit basic_json_reader(Source&& source, const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            default_visitor_,
                            basic_json_decode_options<CharT>(),
                            default_json_parsing(),
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source, 
                      const basic_json_decode_options<CharT>& options, 
                      const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            default_visitor_,
                            options,
                            default_json_parsing(),
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            default_visitor_,
                            basic_json_decode_options<CharT>(),
                            err_handler,
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source, 
                      const basic_json_decode_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            default_visitor_,
                            options,
                            err_handler,
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source, 
                      basic_json_visitor<CharT>& visitor, 
                      const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            visitor,
                            basic_json_decode_options<CharT>(),
                            default_json_parsing(),
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source, 
                      basic_json_visitor<CharT>& visitor,
                      const basic_json_decode_options<CharT>& options, 
                      const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            visitor,
                            options,
                            default_json_parsing(),
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source,
                      basic_json_visitor<CharT>& visitor,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const Allocator& alloc = Allocator())
        : basic_json_reader(std::forward<Source>(source),
                            visitor,
                            basic_json_decode_options<CharT>(),
                            err_handler,
                            alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source,
                      basic_json_visitor<CharT>& visitor, 
                      const basic_json_decode_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const Allocator& alloc = Allocator(),
                      typename std::enable_if<!std::is_constructible<jsoncons::basic_string_view<CharT>,Source>::value>::type* = 0)
       : visitor_(visitor),
         parser_(options,err_handler,alloc),
         source_(std::forward<Source>(source)),
         buffer_reader_(default_max_buffer_length, alloc)
    {
    }

    template <class Source>
    basic_json_reader(Source&& source,
                      basic_json_visitor<CharT>& visitor, 
                      const basic_json_decode_options<CharT>& options,
                      std::function<bool(json_errc,const ser_context&)> err_handler, 
                      const Allocator& alloc = Allocator(),
                      typename std::enable_if<std::is_constructible<jsoncons::basic_string_view<CharT>,Source>::value>::type* = 0)
       : visitor_(visitor),
         parser_(options,err_handler,alloc),
         buffer_reader_(0, alloc)
    {
        jsoncons::basic_string_view<CharT> sv(std::forward<Source>(source));

        auto r = unicode_traits::detect_json_encoding(sv.data(), sv.size());
        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
        {
            JSONCONS_THROW(ser_error(json_errc::illegal_unicode_character,parser_.line(),parser_.column()));
        }
        std::size_t offset = (r.ptr - sv.data());
        parser_.update(sv.data()+offset,sv.size()-offset);
    }

    std::size_t buffer_length() const
    {
        return buffer_reader_.buffer_length();
    }

    void buffer_length(std::size_t size)
    {
        buffer_reader_.buffer_length(size);
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use max_nesting_depth() on options")
    int max_nesting_depth() const
    {
        return parser_.max_nesting_depth();
    }

    JSONCONS_DEPRECATED_MSG("Instead, use max_nesting_depth(int) on options")
    void max_nesting_depth(int depth)
    {
        parser_.max_nesting_depth(depth);
    }
#endif
    void read_next()
    {
        std::error_code ec;
        read_next(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read_next(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = json_errc::source_error;
            return;
        }        
        parser_.reset();
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
        
        while (!buffer_reader_.eof())
        {
            parser_.skip_whitespace();
            if (parser_.source_exhausted())
            {
                buffer_reader_.read(source_, ec);
                if (ec) return;
                if (!buffer_reader_.eof())
                {
                    parser_.update(buffer_reader_.data(),buffer_reader_.length());
                }
            }
            else
            {
                break;
            }
        }
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
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

    void check_done(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = json_errc::source_error;
            return;
        }   
        if (buffer_reader_.eof())
        {
            parser_.check_done(ec);
            if (ec) return;
        }
        else
        {
            while (!buffer_reader_.eof())
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
                if (!buffer_reader_.eof())
                {
                    parser_.check_done(ec);
                    if (ec) return;
                }
            }
        }
    }

    bool eof() const
    {
        return buffer_reader_.eof();
    }

    void read()
    {
        read_next();
        check_done();
    }

    void read(std::error_code& ec)
    {
        read_next(ec);
        if (!ec)
        {
            check_done(ec);
        }
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    JSONCONS_DEPRECATED_MSG("Instead, use buffer_length()")
    std::size_t buffer_capacity() const
    {
        return buffer_reader_.buffer_length();
    }

    JSONCONS_DEPRECATED_MSG("Instead, use buffer_length(std::size_t)")
    void buffer_capacity(std::size_t length)
    {
        buffer_reader_.buffer_length(length);
    }
#endif

};

using json_reader = basic_json_reader<char>;
using wjson_reader = basic_json_reader<wchar_t>;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use json_reader") typedef json_reader json_string_reader;
JSONCONS_DEPRECATED_MSG("Instead, use wjson_reader") typedef wjson_reader wjson_string_reader;
#endif

}

#endif

