// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BSON_BSON_PARSER_HPP
#define JSONCONS_BSON_BSON_PARSER_HPP

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
#include <jsoncons_ext/bson/bson_options.hpp>

namespace jsoncons { namespace bson {

enum class parse_mode {root,before_done,document,array,value};

struct parse_state 
{
    parse_mode mode; 
    std::size_t length;
    uint8_t type;
    std::size_t index;

    parse_state(parse_mode mode, std::size_t length, uint8_t type = 0) noexcept
        : mode(mode), length(length), type(type), index(0)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
    parse_state& operator=(const parse_state&) = default;
    parse_state& operator=(parse_state&&) = default;
};

template <class Src,class Allocator=std::allocator<char>>
class basic_bson_parser : public ser_context
{
    using char_type = char;
    using char_traits_type = std::char_traits<char>;
    using temp_allocator_type = Allocator;
    using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<char_type>;                  
    using byte_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<uint8_t>;                  
    using parse_state_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<parse_state>;                         

    Src source_;
    bson_decode_options options_;
    bool more_;
    bool done_;
    std::basic_string<char,std::char_traits<char>,char_allocator_type> text_buffer_;
    std::vector<parse_state,parse_state_allocator_type> state_stack_;
    int nesting_depth_;
public:
    template <class Source>
    basic_bson_parser(Source&& source,
                      const bson_decode_options& options = bson_decode_options(),
                      const Allocator alloc = Allocator())
       : source_(std::forward<Source>(source)), 
         options_(options),
         more_(true), 
         done_(false),
         text_buffer_(alloc),
         state_stack_(alloc),
         nesting_depth_(0)

    {
        state_stack_.emplace_back(parse_mode::root,0);
    }

    void restart()
    {
        more_ = true;
    }

    void reset()
    {
        state_stack_.clear();
        state_stack_.emplace_back(parse_mode::root,0);
        more_ = true;
        done_ = false;
    }

    bool done() const
    {
        return done_;
    }

    bool stopped() const
    {
        return !more_;
    }

    std::size_t line() const override
    {
        return 0;
    }

    std::size_t column() const override
    {
        return source_.position();
    }

    void array_expected(json_visitor& visitor, std::error_code& ec)
    {
        if (state_stack_.size() == 2 && state_stack_.back().mode == parse_mode::document)
        {
            state_stack_.back().mode = parse_mode::array;
            more_ = visitor.begin_array(semantic_tag::none, *this, ec);
        }
    }

    void parse(json_visitor& visitor, std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = bson_errc::source_error;
            more_ = false;
            return;
        }
        
        while (!done_ && more_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::root:
                    state_stack_.back().mode = parse_mode::before_done;
                    begin_document(visitor, ec);
                    break;
                case parse_mode::document:
                {
                    auto t = source_.get_character();
                    if (!t)
                    {
                        ec = bson_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    if (t.value() != 0x00)
                    {
                        read_e_name(visitor,jsoncons::bson::detail::bson_container_type::document,ec);
                        state_stack_.back().mode = parse_mode::value;
                        state_stack_.back().type = t.value();
                    }
                    else
                    {
                        end_document(visitor,ec);
                    }
                    break;
                }
                case parse_mode::array:
                {
                    auto t = source_.get_character();
                    if (!t)
                    {
                        ec = bson_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    if (t.value() != 0x00)
                    {
                        read_e_name(visitor,jsoncons::bson::detail::bson_container_type::array,ec);
                        read_value(visitor, t.value(), ec);
                    }
                    else
                    {
                        end_array(visitor,ec);
                    }
                    break;
                }
                case parse_mode::value:
                    state_stack_.back().mode = parse_mode::document;
                    read_value(visitor,state_stack_.back().type,ec);
                    break;
                case parse_mode::before_done:
                {
                    JSONCONS_ASSERT(state_stack_.size() == 1);
                    state_stack_.clear();
                    more_ = false;
                    done_ = true;
                    visitor.flush();
                    break;
                }
            }
        }
    }

private:

    void begin_document(json_visitor& visitor, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
        {
            ec = bson_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 
        uint8_t buf[sizeof(int32_t)]; 
        if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
        {
            ec = bson_errc::unexpected_eof;
            more_ = false;
            return;
        }
        auto length = binary::little_to_native<int32_t>(buf, sizeof(buf));

        more_ = visitor.begin_object(semantic_tag::none, *this, ec);
        state_stack_.emplace_back(parse_mode::document,length);
    }

    void end_document(json_visitor& visitor, std::error_code& ec)
    {
        --nesting_depth_;
        more_ = visitor.end_object(*this,ec);
        state_stack_.pop_back();
    }

    void begin_array(json_visitor& visitor, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
        {
            ec = bson_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 
        uint8_t buf[sizeof(int32_t)]; 
        if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
        {
            ec = bson_errc::unexpected_eof;
            more_ = false;
            return;
        }
        /* auto len = */ binary::little_to_native<int32_t>(buf, sizeof(buf));

        more_ = visitor.begin_array(semantic_tag::none, *this, ec);
        state_stack_.emplace_back(parse_mode::array,0);
    }

    void end_array(json_visitor& visitor, std::error_code& ec)
    {
        --nesting_depth_;

        more_ = visitor.end_array(*this, ec);
        state_stack_.pop_back();
    }

    void read_e_name(json_visitor& visitor, jsoncons::bson::detail::bson_container_type type, std::error_code& ec)
    {
        text_buffer_.clear();
        character_result<typename Src::value_type> c;
        while ((c = source_.get_character()) && c.value() != 0)
        {
            text_buffer_.push_back(c.value());
        }
        if (type == jsoncons::bson::detail::bson_container_type::document)
        {
            auto result = unicode_traits::validate(text_buffer_.data(),text_buffer_.size());
            if (result.ec != unicode_traits::conv_errc())
            {
                ec = bson_errc::invalid_utf8_text_string;
                more_ = false;
                return;
            }
            more_ = visitor.key(jsoncons::basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this, ec);
        }
    }

    void read_value(json_visitor& visitor, uint8_t type, std::error_code& ec)
    {
        switch (type)
        {
            case jsoncons::bson::detail::bson_format::double_cd:
            {
                uint8_t buf[sizeof(double)]; 
                if (source_.read(buf, sizeof(double)) != sizeof(double))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                double res = binary::little_to_native<double>(buf, sizeof(buf));
                more_ = visitor.double_value(res, semantic_tag::none, *this, ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::string_cd:
            {
                uint8_t buf[sizeof(int32_t)]; 
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto len = binary::little_to_native<int32_t>(buf, sizeof(buf));
                if (len < 1)
                {
                    ec = bson_errc::string_length_is_non_positive;
                    more_ = false;
                    return;
                }

                std::vector<char> s;
                std::size_t size = static_cast<std::size_t>(len) - static_cast<std::size_t>(1);
                if (source_reader<Src>::read(source_,s,size) != size)
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto c = source_.get_character();
                if (!c) // discard 0
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto result = unicode_traits::validate(s.data(), s.size());
                if (result.ec != unicode_traits::conv_errc())
                {
                    ec = bson_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                more_ = visitor.string_value(jsoncons::basic_string_view<char>(s.data(),s.size()), semantic_tag::none, *this, ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::document_cd: 
            {
                begin_document(visitor,ec);
                break;
            }

            case jsoncons::bson::detail::bson_format::array_cd: 
            {
                begin_array(visitor,ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::null_cd: 
            {
                more_ = visitor.null_value(semantic_tag::none, *this, ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::bool_cd:
            {
                auto val = source_.get_character();
                if (!val)
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                more_ = visitor.bool_value(val.value() != 0, semantic_tag::none, *this, ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::int32_cd: 
            {
                uint8_t buf[sizeof(int32_t)]; 
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto val = binary::little_to_native<int32_t>(buf, sizeof(buf));
                more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
                break;
            }

            case jsoncons::bson::detail::bson_format::timestamp_cd: 
            {
                uint8_t buf[sizeof(uint64_t)]; 
                if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto val = binary::little_to_native<uint64_t>(buf, sizeof(buf));
                more_ = visitor.uint64_value(val, semantic_tag::none, *this, ec);
                break;
            }

            case jsoncons::bson::detail::bson_format::int64_cd: 
            {
                uint8_t buf[sizeof(int64_t)]; 
                if (source_.read(buf, sizeof(int64_t)) != sizeof(int64_t))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto val = binary::little_to_native<int64_t>(buf, sizeof(buf));
                more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
                break;
            }

            case jsoncons::bson::detail::bson_format::datetime_cd: 
            {
                uint8_t buf[sizeof(int64_t)]; 
                if (source_.read(buf, sizeof(int64_t)) != sizeof(int64_t))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                auto val = binary::little_to_native<int64_t>(buf, sizeof(buf));
                more_ = visitor.int64_value(val, semantic_tag::epoch_milli, *this, ec);
                break;
            }
            case jsoncons::bson::detail::bson_format::binary_cd: 
            {
                uint8_t buf[sizeof(int32_t)]; 
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                const auto len = binary::little_to_native<int32_t>(buf, sizeof(buf));
                if (len < 0)
                {
                    ec = bson_errc::length_is_negative;
                    more_ = false;
                    return;
                }
                auto subtype = source_.get_character();
                if (!subtype)
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }

                std::vector<uint8_t> v;
                if (source_reader<Src>::read(source_, v, len) != static_cast<std::size_t>(len))
                {
                    ec = bson_errc::unexpected_eof;
                    more_ = false;
                    return;
                }

                more_ = visitor.byte_string_value(byte_string_view(v), 
                                                  subtype.value(), 
                                                  *this,
                                                  ec);
                break;
            }
            default:
            {
                ec = bson_errc::unknown_type;
                more_ = false;
                return;
            }
        }

    }
};

}}

#endif
