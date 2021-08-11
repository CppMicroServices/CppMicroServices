// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_MSGPACK_PARSER_HPP
#define JSONCONS_MSGPACK_MSGPACK_PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/bigint.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons_ext/msgpack/msgpack_detail.hpp>
#include <jsoncons_ext/msgpack/msgpack_error.hpp>
#include <jsoncons_ext/msgpack/msgpack_options.hpp>
#include <jsoncons/json_visitor2.hpp>

namespace jsoncons { namespace msgpack {

enum class parse_mode {root,before_done,array,map_key,map_value};

struct parse_state 
{
    parse_mode mode; 
    std::size_t length;
    std::size_t index;

    parse_state(parse_mode mode, std::size_t length) noexcept
        : mode(mode), length(length), index(0)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
};

template <class Src,class Allocator=std::allocator<char>>
class basic_msgpack_parser : public ser_context
{
    using char_type = char;
    using char_traits_type = std::char_traits<char>;
    using temp_allocator_type = Allocator;
    using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<char_type>;                  
    using byte_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<uint8_t>;                  
    using int64_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<int64_t>;                  
    using parse_state_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<parse_state>;                         

    static constexpr int64_t nanos_in_second = 1000000000;

    Src source_;
    msgpack_decode_options options_;
    bool more_;
    bool done_;
    std::basic_string<char,std::char_traits<char>,char_allocator_type> text_buffer_;
    std::vector<uint8_t,byte_allocator_type> bytes_buffer_;
    std::vector<parse_state,parse_state_allocator_type> state_stack_;
    int nesting_depth_;

public:
    template <class Source>
    basic_msgpack_parser(Source&& source,
                         const msgpack_decode_options& options = msgpack_decode_options(),
                         const Allocator alloc = Allocator())
       : source_(std::forward<Source>(source)),
         options_(options),
         more_(true), 
         done_(false),
         text_buffer_(alloc),
         bytes_buffer_(alloc),
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

    void parse(json_visitor2& visitor, std::error_code& ec)
    {
        while (!done_ && more_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::array:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_item(visitor, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        end_array(visitor, ec);
                    }
                    break;
                }
                case parse_mode::map_key:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        state_stack_.back().mode = parse_mode::map_value;
                        read_item(visitor, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        end_object(visitor, ec);
                    }
                    break;
                }
                case parse_mode::map_value:
                {
                    state_stack_.back().mode = parse_mode::map_key;
                    read_item(visitor, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::root:
                {
                    state_stack_.back().mode = parse_mode::before_done;
                    read_item(visitor, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
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

    void read_item(json_visitor2& visitor, std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = msgpack_errc::source_error;
            more_ = false;
            return;
        }   

        auto ch = source_.get_character();
        if (!ch)
        {
            ec = msgpack_errc::unexpected_eof;
            more_ = false;
            return;
        }
        uint8_t type = ch.value();

        if (type <= 0xbf)
        {
            if (type <= 0x7f) 
            {
                // positive fixint
                more_ = visitor.uint64_value(type, semantic_tag::none, *this, ec);
            }
            else if (type <= 0x8f) 
            {
                begin_object(visitor,type,ec); // fixmap
            }
            else if (type <= 0x9f) 
            {
                begin_array(visitor,type,ec); // fixarray
            }
            else 
            {
                // fixstr
                const size_t len = type & 0x1f;

                text_buffer_.clear();

                if (source_reader<Src>::read(source_,text_buffer_,len) != static_cast<std::size_t>(len))
                {
                    ec = msgpack_errc::unexpected_eof;
                    more_ = false;
                    return;
                }

                auto result = unicode_traits::validate(text_buffer_.data(),text_buffer_.size());
                if (result.ec != unicode_traits::conv_errc())
                {
                    ec = msgpack_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                more_ = visitor.string_value(jsoncons::basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), semantic_tag::none, *this, ec);
            }
        }
        else if (type >= 0xe0) 
        {
            // negative fixint
            more_ = visitor.int64_value(static_cast<int8_t>(type), semantic_tag::none, *this, ec);
        }
        else
        {
            switch (type)
            {
                case jsoncons::msgpack::detail::msgpack_format::nil_cd: 
                {
                    more_ = visitor.null_value(semantic_tag::none, *this, ec);
                    break;
                }
                case jsoncons::msgpack::detail::msgpack_format::true_cd:
                {
                    more_ = visitor.bool_value(true, semantic_tag::none, *this, ec);
                    break;
                }
                case jsoncons::msgpack::detail::msgpack_format::false_cd:
                {
                    more_ = visitor.bool_value(false, semantic_tag::none, *this, ec);
                    break;
                }
                case jsoncons::msgpack::detail::msgpack_format::float32_cd: 
                {
                    uint8_t buf[sizeof(float)];
                    if (source_.read(buf, sizeof(float)) != sizeof(float))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    float val = binary::big_to_native<float>(buf, sizeof(buf));
                    more_ = visitor.double_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::float64_cd: 
                {
                    uint8_t buf[sizeof(double)];
                    if (source_.read(buf, sizeof(double)) != sizeof(double))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    double val = binary::big_to_native<double>(buf, sizeof(buf));
                    more_ = visitor.double_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::uint8_cd: 
                {
                    auto val = source_.get_character();
                    if (!val)
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    more_ = visitor.uint64_value(val.value(), semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::uint16_cd: 
                {
                    uint8_t buf[sizeof(uint16_t)];
                    if (source_.read(buf, sizeof(uint16_t)) !=sizeof(uint16_t)) 
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    uint16_t val = binary::big_to_native<uint16_t>(buf, sizeof(buf));
                    more_ = visitor.uint64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::uint32_cd: 
                {
                    uint8_t buf[sizeof(uint32_t)];
                    if (source_.read(buf, sizeof(uint32_t)) != sizeof(uint32_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    uint32_t val = binary::big_to_native<uint32_t>(buf, sizeof(buf));
                    more_ = visitor.uint64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::uint64_cd: 
                {
                    uint8_t buf[sizeof(uint64_t)];
                    if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    uint64_t val = binary::big_to_native<uint64_t>(buf, sizeof(buf));
                    more_ = visitor.uint64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::int8_cd: 
                {
                    uint8_t buf[sizeof(int8_t)];
                    if (source_.read(buf, sizeof(int8_t)) != sizeof(int8_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    int8_t val = binary::big_to_native<int8_t>(buf, sizeof(buf));
                    more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::int16_cd: 
                {
                    uint8_t buf[sizeof(int16_t)];
                    if (source_.read(buf, sizeof(int16_t)) != sizeof(int16_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    int16_t val = binary::big_to_native<int16_t>(buf, sizeof(buf));
                    more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::int32_cd: 
                {
                    uint8_t buf[sizeof(int32_t)];
                    if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    int32_t val = binary::big_to_native<int32_t>(buf, sizeof(buf));
                    more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::int64_cd: 
                {
                    uint8_t buf[sizeof(int64_t)];
                    if (source_.read(buf, sizeof(int64_t)) != sizeof(int64_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    int64_t val = binary::big_to_native<int64_t>(buf, sizeof(buf));
                    more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::str8_cd: 
                case jsoncons::msgpack::detail::msgpack_format::str16_cd: 
                case jsoncons::msgpack::detail::msgpack_format::str32_cd: 
                {
                    std::size_t len = get_size(type, ec);
                    if (!more_)
                    {
                        return;
                    }

                    text_buffer_.clear();
                    if (source_reader<Src>::read(source_,text_buffer_,len) != static_cast<std::size_t>(len))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }

                    auto result = unicode_traits::validate(text_buffer_.data(),text_buffer_.size());
                    if (result.ec != unicode_traits::conv_errc())
                    {
                        ec = msgpack_errc::invalid_utf8_text_string;
                        more_ = false;
                        return;
                    }
                    more_ = visitor.string_value(jsoncons::basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), semantic_tag::none, *this, ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::bin8_cd: 
                case jsoncons::msgpack::detail::msgpack_format::bin16_cd: 
                case jsoncons::msgpack::detail::msgpack_format::bin32_cd: 
                {
                    std::size_t len = get_size(type,ec);
                    if (!more_)
                    {
                        return;
                    }
                    bytes_buffer_.clear();
                    if (source_reader<Src>::read(source_,bytes_buffer_,len) != static_cast<std::size_t>(len))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }

                    more_ = visitor.byte_string_value(byte_string_view(bytes_buffer_.data(),bytes_buffer_.size()), 
                                                      semantic_tag::none, 
                                                      *this,
                                                      ec);
                    break;
                }
                case jsoncons::msgpack::detail::msgpack_format::fixext1_cd: 
                case jsoncons::msgpack::detail::msgpack_format::fixext2_cd: 
                case jsoncons::msgpack::detail::msgpack_format::fixext4_cd: 
                case jsoncons::msgpack::detail::msgpack_format::fixext8_cd: 
                case jsoncons::msgpack::detail::msgpack_format::fixext16_cd: 
                case jsoncons::msgpack::detail::msgpack_format::ext8_cd: 
                case jsoncons::msgpack::detail::msgpack_format::ext16_cd: 
                case jsoncons::msgpack::detail::msgpack_format::ext32_cd: 
                {
                    std::size_t len = get_size(type,ec);
                    if (!more_)
                    {
                        return;
                    }

                    // type
                    uint8_t buf[sizeof(int8_t)];
                    if (source_.read(buf, sizeof(int8_t)) != sizeof(int8_t))
                    {
                        ec = msgpack_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }

                    int8_t ext_type = binary::big_to_native<int8_t>(buf, sizeof(buf));

                    bool is_timestamp = false; 
                    if (ext_type == -1)
                    {
                        is_timestamp = true;;
                    }

                    // payload
                    if (is_timestamp && len == 4)
                    {
                        uint8_t buf32[sizeof(uint32_t)];
                        if (source_.read(buf32, sizeof(uint32_t)) != sizeof(uint32_t))
                        {
                            ec = msgpack_errc::unexpected_eof;
                            more_ = false;
                            return;
                        }
                        uint32_t val = binary::big_to_native<uint32_t>(buf32, sizeof(buf32));
                        more_ = visitor.uint64_value(val, semantic_tag::epoch_second, *this, ec);
                    }
                    else if (is_timestamp && len == 8)
                    {
                        uint8_t buf64[sizeof(uint64_t)];
                        if (source_.read(buf64, sizeof(uint64_t)) != sizeof(uint64_t))
                        {
                            ec = msgpack_errc::unexpected_eof;
                            more_ = false;
                            return;
                        }
                        uint64_t data64 = binary::big_to_native<uint64_t>(buf64, sizeof(buf64));
                        uint64_t sec = data64 & 0x00000003ffffffffL;
                        uint64_t nsec = data64 >> 34;

                        bigint nano(sec);
                        nano *= uint64_t(nanos_in_second);
                        nano += nsec;
                        text_buffer_.clear();
                        nano.write_string(text_buffer_);
                        more_ = visitor.string_value(text_buffer_, semantic_tag::epoch_nano, *this, ec);
                        if (!more_) return;
                    }
                    else if (is_timestamp && len == 12)
                    {
                        uint8_t buf1[sizeof(uint32_t)];
                        if (source_.read(buf1, sizeof(uint32_t)) != sizeof(uint32_t))
                        {
                            ec = msgpack_errc::unexpected_eof;
                            more_ = false;
                            return;
                        }
                        uint32_t nsec = binary::big_to_native<uint32_t>(buf1, sizeof(buf1));

                        uint8_t buf2[sizeof(int64_t)];
                        if (source_.read(buf2, sizeof(int64_t)) != sizeof(int64_t))
                        {
                            ec = msgpack_errc::unexpected_eof;
                            more_ = false;
                            return;
                        }
                        int64_t sec = binary::big_to_native<int64_t>(buf2, sizeof(buf2));

                        bigint nano(sec);

                        nano *= uint64_t(nanos_in_second);

                        if (nano < 0)
                        {
                            nano -= nsec;
                        }
                        else
                        {
                            nano += nsec;
                        }

                        text_buffer_.clear();
                        nano.write_string(text_buffer_);
                        more_ = visitor.string_value(text_buffer_, semantic_tag::epoch_nano, *this, ec);
                        if (!more_) return;
                    }
                    else
                    {
                        bytes_buffer_.clear();
                        if (source_reader<Src>::read(source_,bytes_buffer_,len) != static_cast<std::size_t>(len))
                        {
                            ec = msgpack_errc::unexpected_eof;
                            more_ = false;
                            return;
                        }

                        more_ = visitor.byte_string_value(byte_string_view(bytes_buffer_.data(),bytes_buffer_.size()), 
                                                          static_cast<uint8_t>(ext_type), 
                                                          *this,
                                                          ec);
                    }
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::array16_cd: 
                case jsoncons::msgpack::detail::msgpack_format::array32_cd: 
                {
                    begin_array(visitor,type,ec);
                    break;
                }

                case jsoncons::msgpack::detail::msgpack_format::map16_cd : 
                case jsoncons::msgpack::detail::msgpack_format::map32_cd : 
                {
                    begin_object(visitor, type, ec);
                    break;
                }

                default:
                {
                    ec = msgpack_errc::unknown_type;
                    more_ = false;
                    return;
                }
            }
        }
    }

    void begin_array(json_visitor2& visitor, uint8_t type, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
        {
            ec = msgpack_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 
        std::size_t length = get_size(type, ec);
        if (!more_)
        {
            return;
        }
        state_stack_.emplace_back(parse_mode::array,length);
        more_ = visitor.begin_array(length, semantic_tag::none, *this, ec);
    }

    void end_array(json_visitor2& visitor, std::error_code& ec)
    {
        --nesting_depth_;

        more_ = visitor.end_array(*this, ec);
        state_stack_.pop_back();
    }

    void begin_object(json_visitor2& visitor, uint8_t type, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
        {
            ec = msgpack_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 
        std::size_t length = get_size(type, ec);
        if (!more_)
        {
            return;
        }
        state_stack_.emplace_back(parse_mode::map_key,length);
        more_ = visitor.begin_object(length, semantic_tag::none, *this, ec);
    }

    void end_object(json_visitor2& visitor, std::error_code& ec)
    {
        --nesting_depth_;
        more_ = visitor.end_object(*this, ec);
        state_stack_.pop_back();
    }

    std::size_t get_size(uint8_t type, std::error_code& ec)
    {
        switch (type)
        {
            case jsoncons::msgpack::detail::msgpack_format::str8_cd: 
            case jsoncons::msgpack::detail::msgpack_format::bin8_cd: 
            case jsoncons::msgpack::detail::msgpack_format::ext8_cd: 
            {
                uint8_t buf[sizeof(int8_t)];
                if (source_.read(buf, sizeof(int8_t)) != sizeof(int8_t))
                {
                    ec = msgpack_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                uint8_t len = binary::big_to_native<uint8_t>(buf, sizeof(buf));
                return static_cast<std::size_t>(len);
            }

            case jsoncons::msgpack::detail::msgpack_format::str16_cd: 
            case jsoncons::msgpack::detail::msgpack_format::bin16_cd: 
            case jsoncons::msgpack::detail::msgpack_format::ext16_cd: 
            case jsoncons::msgpack::detail::msgpack_format::array16_cd: 
            case jsoncons::msgpack::detail::msgpack_format::map16_cd:
            {
                uint8_t buf[sizeof(int16_t)];
                if (source_.read(buf, sizeof(int16_t)) != sizeof(int16_t))
                {
                    ec = msgpack_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                uint16_t len = binary::big_to_native<uint16_t>(buf, sizeof(buf));
                return static_cast<std::size_t>(len);
            }

            case jsoncons::msgpack::detail::msgpack_format::str32_cd: 
            case jsoncons::msgpack::detail::msgpack_format::bin32_cd: 
            case jsoncons::msgpack::detail::msgpack_format::ext32_cd: 
            case jsoncons::msgpack::detail::msgpack_format::array32_cd: 
            case jsoncons::msgpack::detail::msgpack_format::map32_cd : 
            {
                uint8_t buf[sizeof(int32_t)];
                if (source_.read(buf, sizeof(int32_t)) != sizeof(int32_t))
                {
                    ec = msgpack_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                uint32_t len = binary::big_to_native<uint32_t>(buf, sizeof(buf));
                return static_cast<std::size_t>(len);
            }
            case jsoncons::msgpack::detail::msgpack_format::fixext1_cd: 
                return 1;
            case jsoncons::msgpack::detail::msgpack_format::fixext2_cd: 
                return 2;
            case jsoncons::msgpack::detail::msgpack_format::fixext4_cd: 
                return 4;
            case jsoncons::msgpack::detail::msgpack_format::fixext8_cd: 
                return 8;
            case jsoncons::msgpack::detail::msgpack_format::fixext16_cd: 
                return 16;
            default:
                if ((type > 0x8f && type <= 0x9f) // fixarray
                    || (type > 0x7f && type <= 0x8f) // fixmap
                   )
                {
                    return type & 0x0f;
                }
                else
                {
                    ec = msgpack_errc::unknown_type;
                    more_ = false;
                    return 0;
                }
                break;
        }
    }
};

}}

#endif
