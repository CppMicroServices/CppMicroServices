// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_MSGPACK_MSGPACK_ENCODER_HPP
#define JSONCONS_MSGPACK_MSGPACK_ENCODER_HPP

#include <string>
#include <vector>
#include <limits> // std::numeric_limits
#include <memory>
#include <utility> // std::move
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/sink.hpp>
#include <jsoncons/detail/parse_number.hpp>
#include <jsoncons_ext/msgpack/msgpack_detail.hpp>
#include <jsoncons_ext/msgpack/msgpack_error.hpp>
#include <jsoncons_ext/msgpack/msgpack_options.hpp>

namespace jsoncons { 
namespace msgpack {

    enum class msgpack_container_type {object, array};

    template<class Sink=jsoncons::binary_stream_sink,class Allocator=std::allocator<char>>
    class basic_msgpack_encoder final : public basic_json_visitor<char>
    {
        enum class decimal_parse_state { start, integer, exp1, exp2, fraction1 };

        static constexpr int64_t nanos_in_milli = 1000000;
        static constexpr int64_t nanos_in_second = 1000000000;
        static constexpr int64_t millis_in_second = 1000;
    public:
        using allocator_type = Allocator;
        using char_type = char;
        using typename basic_json_visitor<char>::string_view_type;
        using sink_type = Sink;

    private:
        struct stack_item
        {
            msgpack_container_type type_;
            std::size_t length_;
            std::size_t count_;

            stack_item(msgpack_container_type type, std::size_t length = 0) noexcept
               : type_(type), length_(length), count_(0)
            {
            }

            std::size_t length() const
            {
                return length_;
            }

            std::size_t count() const
            {
                return count_;
            }

            bool is_object() const
            {
                return type_ == msgpack_container_type::object;
            }
        };

        Sink sink_;
        const msgpack_encode_options options_;
        allocator_type alloc_;

        std::vector<stack_item> stack_;
        int nesting_depth_;

        // Noncopyable and nonmoveable
        basic_msgpack_encoder(const basic_msgpack_encoder&) = delete;
        basic_msgpack_encoder& operator=(const basic_msgpack_encoder&) = delete;
    public:
        explicit basic_msgpack_encoder(Sink&& sink, 
                                       const Allocator& alloc = Allocator())
           : basic_msgpack_encoder(std::forward<Sink>(sink), msgpack_encode_options(), alloc)
        {
        }

        explicit basic_msgpack_encoder(Sink&& sink, 
                                       const msgpack_encode_options& options, 
                                       const Allocator& alloc = Allocator())
           : sink_(std::forward<Sink>(sink)),
             options_(options),
             alloc_(alloc),
             nesting_depth_(0)
        {
        }

        ~basic_msgpack_encoder() noexcept
        {
            sink_.flush();
        }

    private:
        // Implementing methods

        void visit_flush() override
        {
            sink_.flush();
        }

        bool visit_begin_object(semantic_tag, const ser_context&, std::error_code& ec) override
        {
            ec = msgpack_errc::object_length_required;
            return false;
        }

        bool visit_begin_object(std::size_t length, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
            {
                ec = msgpack_errc::max_nesting_depth_exceeded;
                return false;
            } 
            stack_.emplace_back(msgpack_container_type::object, length);

            if (length <= 15)
            {
                // fixmap
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixmap_base_cd | (length & 0xf));
            }
            else if (length <= 65535)
            {
                // map 16
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::map16_cd);
                binary::native_to_big(static_cast<uint16_t>(length), 
                                      std::back_inserter(sink_));
            }
            else if (length <= 4294967295)
            {
                // map 32
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::map32_cd);
                binary::native_to_big(static_cast<uint32_t>(length),
                                      std::back_inserter(sink_));
            }

            return true;
        }

        bool visit_end_object(const ser_context&, std::error_code& ec) override
        {
            JSONCONS_ASSERT(!stack_.empty());
            --nesting_depth_;

            if (stack_.back().count() < stack_.back().length())
            {
                ec = msgpack_errc::too_few_items;
                return false;
            }
            else if (stack_.back().count() > stack_.back().length())
            {
                ec = msgpack_errc::too_many_items;
                return false;
            }

            stack_.pop_back();
            end_value();
            return true;
        }

        bool visit_begin_array(semantic_tag, const ser_context&, std::error_code& ec) override
        {
            ec = msgpack_errc::array_length_required;
            return false;
        }

        bool visit_begin_array(std::size_t length, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
            {
                ec = msgpack_errc::max_nesting_depth_exceeded;
                return false;
            } 
            stack_.emplace_back(msgpack_container_type::array, length);
            if (length <= 15)
            {
                // fixarray
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixarray_base_cd | (length & 0xf));
            }
            else if (length <= (std::numeric_limits<uint16_t>::max)())
            {
                // array 16
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::array16_cd);
                binary::native_to_big(static_cast<uint16_t>(length),std::back_inserter(sink_));
            }
            else if (length <= (std::numeric_limits<uint32_t>::max)())
            {
                // array 32
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::array32_cd);
                binary::native_to_big(static_cast<uint32_t>(length),std::back_inserter(sink_));
            }
            return true;
        }

        bool visit_end_array(const ser_context&, std::error_code& ec) override
        {
            JSONCONS_ASSERT(!stack_.empty());

            --nesting_depth_;

            if (stack_.back().count() < stack_.back().length())
            {
                ec = msgpack_errc::too_few_items;
                return false;
            }
            else if (stack_.back().count() > stack_.back().length())
            {
                ec = msgpack_errc::too_many_items;
                return false;
            }

            stack_.pop_back();
            end_value();
            return true;
        }

        bool visit_key(const string_view_type& name, const ser_context&, std::error_code&) override
        {
            write_string_value(name);
            return true;
        }

        bool visit_null(semantic_tag, const ser_context&, std::error_code&) override
        {
            // nil
            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::nil_cd);
            end_value();
            return true;
        }

        void write_timestamp(int64_t seconds, int64_t nanoseconds)
        {
            if ((seconds >> 34) == 0) 
            {
                uint64_t data64 = (nanoseconds << 34) | seconds;
                if ((data64 & 0xffffffff00000000L) == 0) 
                {
                    // timestamp 32
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext4_cd);
                    sink_.push_back(0xff);
                    binary::native_to_big(static_cast<uint32_t>(data64), std::back_inserter(sink_));
                }
                else 
                {
                    // timestamp 64
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext8_cd);
                    sink_.push_back(0xff);
                    binary::native_to_big(static_cast<uint64_t>(data64), std::back_inserter(sink_));
                }
            }
            else 
            {
                // timestamp 96
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::ext8_cd);
                sink_.push_back(0x0c); // 12
                sink_.push_back(0xff);
                binary::native_to_big(static_cast<uint32_t>(nanoseconds), std::back_inserter(sink_));
                binary::native_to_big(static_cast<uint64_t>(seconds), std::back_inserter(sink_));
            }
        }

        bool visit_string(const string_view_type& sv, semantic_tag tag, const ser_context&, std::error_code& ec) override
        {
            switch (tag)
            {
                case semantic_tag::epoch_second:
                {
                    auto result = jsoncons::detail::to_integer<int64_t>(sv.data(), sv.length());
                    if (!result)
                    {
                        ec = msgpack_errc::invalid_timestamp;
                        return false;
                    }
                    int64_t seconds = result.value();
                    write_timestamp(seconds, 0);
                    break;
                }
                case semantic_tag::epoch_milli:
                {
                    bigint n = bigint::from_string(sv.data(), sv.length());
                    if (n != 0)
                    {
                        bigint q;
                        bigint rem;
                        n.divide(millis_in_second, q, rem, true);
                        int64_t seconds = static_cast<int64_t>(q);
                        int64_t nanoseconds = static_cast<int64_t>(rem) * nanos_in_milli;
                        if (nanoseconds < 0)
                        {
                            nanoseconds = -nanoseconds; 
                        }
                        write_timestamp(seconds, nanoseconds);
                    }
                    else
                    {
                        write_timestamp(0, 0);
                    }
                    break;
                }
                case semantic_tag::epoch_nano:
                {
                    bigint n = bigint::from_string(sv.data(), sv.length());
                    if (n != 0)
                    {
                        bigint q;
                        bigint rem;
                        n.divide(nanos_in_second, q, rem, true);
                        int64_t seconds = static_cast<int64_t>(q);
                        int64_t nanoseconds = static_cast<int64_t>(rem);
                        if (nanoseconds < 0)
                        {
                            nanoseconds = -nanoseconds; 
                        }
                        write_timestamp(seconds, nanoseconds);
                    }
                    else
                    {
                        write_timestamp(0, 0);
                    }
                    break;
                }
                default:
                {
                    write_string_value(sv);
                    end_value();
                    break;
                }
            }
            return true;
        }

        void write_string_value(const string_view_type& sv) 
        {
            auto sink = unicode_traits::validate(sv.data(), sv.size());
            if (sink.ec != unicode_traits::conv_errc())
            {
                JSONCONS_THROW(ser_error(msgpack_errc::invalid_utf8_text_string));
            }

            const size_t length = sv.length();
            if (length <= 31)
            {
                // fixstr stores a byte array whose length is upto 31 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixstr_base_cd | static_cast<uint8_t>(length));
            }
            else if (length <= (std::numeric_limits<uint8_t>::max)())
            {
                // str 8 stores a byte array whose length is upto (2^8)-1 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::str8_cd);
                sink_.push_back(static_cast<uint8_t>(length));
            }
            else if (length <= (std::numeric_limits<uint16_t>::max)())
            {
                // str 16 stores a byte array whose length is upto (2^16)-1 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::str16_cd);
                binary::native_to_big(static_cast<uint16_t>(length), std::back_inserter(sink_));
            }
            else if (length <= (std::numeric_limits<uint32_t>::max)())
            {
                // str 32 stores a byte array whose length is upto (2^32)-1 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::str32_cd);
                binary::native_to_big(static_cast<uint32_t>(length),std::back_inserter(sink_));
            }

            for (auto c : sv)
            {
                sink_.push_back(c);
            }
        }

        bool visit_byte_string(const byte_string_view& b, 
                               semantic_tag, 
                               const ser_context&,
                               std::error_code&) override
        {

            const std::size_t length = b.size();
            if (length <= (std::numeric_limits<uint8_t>::max)())
            {
                // bin 8 stores a byte array whose length is upto (2^8)-1 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::bin8_cd);
                sink_.push_back(static_cast<uint8_t>(length));
            }
            else if (length <= (std::numeric_limits<uint16_t>::max)())
            {
                // bin 16 stores a byte array whose length is upto (2^16)-1 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::bin16_cd);
                binary::native_to_big(static_cast<uint16_t>(length), std::back_inserter(sink_));
            }
            else if (length <= (std::numeric_limits<uint32_t>::max)())
            {
                // bin 32 stores a byte array whose length is upto (2^32)-1 bytes
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::bin32_cd);
                binary::native_to_big(static_cast<uint32_t>(length),std::back_inserter(sink_));
            }

            for (auto c : b)
            {
                sink_.push_back(c);
            }

            end_value();
            return true;
        }

        bool visit_byte_string(const byte_string_view& b, 
                               uint64_t ext_tag, 
                               const ser_context&,
                               std::error_code&) override
        {
            const std::size_t length = b.size();
            switch (length)
            {
                case 1:
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext1_cd);
                    sink_.push_back(static_cast<uint8_t>(ext_tag));
                    break;
                case 2:
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext2_cd);
                    sink_.push_back(static_cast<uint8_t>(ext_tag));
                    break;
                case 4:
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext4_cd);
                    sink_.push_back(static_cast<uint8_t>(ext_tag));
                    break;
                case 8:
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext8_cd);
                    sink_.push_back(static_cast<uint8_t>(ext_tag));
                    break;
                case 16:
                    sink_.push_back(jsoncons::msgpack::detail::msgpack_format::fixext16_cd);
                    sink_.push_back(static_cast<uint8_t>(ext_tag));
                    break;
                default:
                    if (length <= (std::numeric_limits<uint8_t>::max)())
                    {
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::ext8_cd);
                        sink_.push_back(static_cast<uint8_t>(length));
                        sink_.push_back(static_cast<uint8_t>(ext_tag));
                    }
                    else if (length <= (std::numeric_limits<uint16_t>::max)())
                    {
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::ext16_cd);
                        binary::native_to_big(static_cast<uint16_t>(length), std::back_inserter(sink_));
                        sink_.push_back(static_cast<uint8_t>(ext_tag));
                    }
                    else if (length <= (std::numeric_limits<uint32_t>::max)())
                    {
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::ext32_cd);
                        binary::native_to_big(static_cast<uint32_t>(length),std::back_inserter(sink_));
                        sink_.push_back(static_cast<uint8_t>(ext_tag));
                    }
                    break;
            }

            for (auto c : b)
            {
                sink_.push_back(c);
            }

            end_value();
            return true;
        }

        bool visit_double(double val, 
                             semantic_tag,
                             const ser_context&,
                             std::error_code&) override
        {
            float valf = (float)val;
            if ((double)valf == val)
            {
                // float 32
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::float32_cd);
                binary::native_to_big(valf,std::back_inserter(sink_));
            }
            else
            {
                // float 64
                sink_.push_back(jsoncons::msgpack::detail::msgpack_format::float64_cd);
                binary::native_to_big(val,std::back_inserter(sink_));
            }

            // write double

            end_value();
            return true;
        }

        bool visit_int64(int64_t val, 
                         semantic_tag tag, 
                         const ser_context&,
                         std::error_code&) override
        {
            switch (tag)
            {
                case semantic_tag::epoch_second:
                    write_timestamp(val, 0);
                    break;
                case semantic_tag::epoch_milli:
                {
                    if (val != 0)
                    {
                        auto dv = std::div(val,millis_in_second);
                        int64_t seconds = dv.quot;
                        int64_t nanoseconds = dv.rem*nanos_in_milli;
                        if (nanoseconds < 0)
                        {
                            nanoseconds = -nanoseconds; 
                        }
                        write_timestamp(seconds, nanoseconds);
                    }
                    else
                    {
                        write_timestamp(0, 0);
                    }
                    break;
                }
                case semantic_tag::epoch_nano:
                {
                    if (val != 0)
                    {
                        auto dv = std::div(val,static_cast<int64_t>(nanos_in_second));
                        int64_t seconds = dv.quot;
                        int64_t nanoseconds = dv.rem;
                        if (nanoseconds < 0)
                        {
                            nanoseconds = -nanoseconds; 
                        }
                        write_timestamp(seconds, nanoseconds);
                    }
                    else
                    {
                        write_timestamp(0, 0);
                    }
                    break;
                }
                default:
                {
                    if (val >= 0)
                    {
                        if (val <= 0x7f)
                        {
                            // positive fixnum stores 7-bit positive integer
                            sink_.push_back(static_cast<uint8_t>(val));
                        }
                        else if (val <= (std::numeric_limits<uint8_t>::max)())
                        {
                            // uint 8 stores a 8-bit unsigned integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint8_cd);
                            sink_.push_back(static_cast<uint8_t>(val));
                        }
                        else if (val <= (std::numeric_limits<uint16_t>::max)())
                        {
                            // uint 16 stores a 16-bit big-endian unsigned integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint16_cd);
                            binary::native_to_big(static_cast<uint16_t>(val),std::back_inserter(sink_));
                        }
                        else if (val <= (std::numeric_limits<uint32_t>::max)())
                        {
                            // uint 32 stores a 32-bit big-endian unsigned integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint32_cd);
                            binary::native_to_big(static_cast<uint32_t>(val),std::back_inserter(sink_));
                        }
                        else if (val <= (std::numeric_limits<int64_t>::max)())
                        {
                            // int 64 stores a 64-bit big-endian signed integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint64_cd);
                            binary::native_to_big(static_cast<uint64_t>(val),std::back_inserter(sink_));
                        }
                    }
                    else
                    {
                        if (val >= -32)
                        {
                            // negative fixnum stores 5-bit negative integer
                            binary::native_to_big(static_cast<int8_t>(val), std::back_inserter(sink_));
                        }
                        else if (val >= (std::numeric_limits<int8_t>::lowest)())
                        {
                            // int 8 stores a 8-bit signed integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::int8_cd);
                            binary::native_to_big(static_cast<int8_t>(val),std::back_inserter(sink_));
                        }
                        else if (val >= (std::numeric_limits<int16_t>::lowest)())
                        {
                            // int 16 stores a 16-bit big-endian signed integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::int16_cd);
                            binary::native_to_big(static_cast<int16_t>(val),std::back_inserter(sink_));
                        }
                        else if (val >= (std::numeric_limits<int32_t>::lowest)())
                        {
                            // int 32 stores a 32-bit big-endian signed integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::int32_cd);
                            binary::native_to_big(static_cast<int32_t>(val),std::back_inserter(sink_));
                        }
                        else if (val >= (std::numeric_limits<int64_t>::lowest)())
                        {
                            // int 64 stores a 64-bit big-endian signed integer
                            sink_.push_back(jsoncons::msgpack::detail::msgpack_format::int64_cd);
                            binary::native_to_big(static_cast<int64_t>(val),std::back_inserter(sink_));
                        }
                    }
                }
                break;
            }
            end_value();
            return true;
        }

        bool visit_uint64(uint64_t val, 
                          semantic_tag tag, 
                          const ser_context&,
                          std::error_code&) override
        {
            switch (tag)
            {
                case semantic_tag::epoch_second:
                    write_timestamp(static_cast<int64_t>(val), 0);
                    break;
                case semantic_tag::epoch_milli:
                {
                    if (val != 0)
                    {
                        auto dv = std::div(static_cast<int64_t>(val), static_cast<int64_t>(millis_in_second));
                        int64_t seconds = dv.quot;
                        int64_t nanoseconds = dv.rem*nanos_in_milli;
                        if (nanoseconds < 0)
                        {
                            nanoseconds = -nanoseconds; 
                        }
                        write_timestamp(seconds, nanoseconds);
                    }
                    else
                    {
                        write_timestamp(0, 0);
                    }
                    break;
                }
                case semantic_tag::epoch_nano:
                {
                    if (val != 0)
                    {
                        auto dv = std::div(static_cast<int64_t>(val), static_cast<int64_t>(nanos_in_second));
                        int64_t seconds = dv.quot;
                        int64_t nanoseconds = dv.rem;
                        if (nanoseconds < 0)
                        {
                            nanoseconds = -nanoseconds; 
                        }
                        write_timestamp(seconds, nanoseconds);
                    }
                    else
                    {
                        write_timestamp(0, 0);
                    }
                    break;
                }
                default:
                {
                    if (val <= static_cast<uint64_t>((std::numeric_limits<int8_t>::max)()))
                    {
                        // positive fixnum stores 7-bit positive integer
                        sink_.push_back(static_cast<uint8_t>(val));
                    }
                    else if (val <= (std::numeric_limits<uint8_t>::max)())
                    {
                        // uint 8 stores a 8-bit unsigned integer
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint8_cd);
                        sink_.push_back(static_cast<uint8_t>(val));
                    }
                    else if (val <= (std::numeric_limits<uint16_t>::max)())
                    {
                        // uint 16 stores a 16-bit big-endian unsigned integer
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint16_cd);
                        binary::native_to_big(static_cast<uint16_t>(val),std::back_inserter(sink_));
                    }
                    else if (val <= (std::numeric_limits<uint32_t>::max)())
                    {
                        // uint 32 stores a 32-bit big-endian unsigned integer
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint32_cd);
                        binary::native_to_big(static_cast<uint32_t>(val),std::back_inserter(sink_));
                    }
                    else if (val <= (std::numeric_limits<uint64_t>::max)())
                    {
                        // uint 64 stores a 64-bit big-endian unsigned integer
                        sink_.push_back(jsoncons::msgpack::detail::msgpack_format::uint64_cd);
                        binary::native_to_big(static_cast<uint64_t>(val),std::back_inserter(sink_));
                    }
                    break;
                }
            }
            end_value();
            return true;
        }

        bool visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
        {
            // true and false
            sink_.push_back(static_cast<uint8_t>(val ? jsoncons::msgpack::detail::msgpack_format::true_cd : jsoncons::msgpack::detail::msgpack_format::false_cd));

            end_value();
            return true;
        }

        void end_value()
        {
            if (!stack_.empty())
            {
                ++stack_.back().count_;
            }
        }
    };

    using msgpack_stream_encoder = basic_msgpack_encoder<jsoncons::binary_stream_sink>;
    using msgpack_bytes_encoder = basic_msgpack_encoder<jsoncons::bytes_sink<std::vector<uint8_t>>>;

    #if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use msgpack_bytes_encoder") typedef msgpack_bytes_encoder msgpack_bytes_serializer;

    template<class Sink=jsoncons::binary_stream_sink>
    using basic_msgpack_serializer = basic_msgpack_encoder<Sink>; 

    JSONCONS_DEPRECATED_MSG("Instead, use msgpack_stream_encoder") typedef msgpack_stream_encoder msgpack_encoder;
    JSONCONS_DEPRECATED_MSG("Instead, use msgpack_stream_encoder") typedef msgpack_stream_encoder msgpack_serializer;
    JSONCONS_DEPRECATED_MSG("Instead, use msgpack_bytes_encoder") typedef msgpack_bytes_encoder msgpack_buffer_serializer;
    #endif

} // namespace msgpack
} // namespace jsoncons

#endif
