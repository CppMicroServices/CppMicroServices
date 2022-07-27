// Copyright 2018 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_VISITOR2_HPP
#define JSONCONS_JSON_VISITOR2_HPP

#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_encoder.hpp>

namespace jsoncons { 

    template <class CharT, class Allocator = std::allocator<char>>
    class basic_json_visitor2_to_visitor_adaptor;

    template <class CharT>
    class basic_json_visitor2 
    {
        template <class Ch, class Allocator>
        friend class basic_json_visitor2_to_visitor_adaptor;
    public:
        using char_type = CharT;
        using char_traits_type = std::char_traits<char_type>;

        using string_view_type = jsoncons::basic_string_view<char_type,char_traits_type>;

        basic_json_visitor2(basic_json_visitor2&&) = default;

        basic_json_visitor2& operator=(basic_json_visitor2&&) = default;

        basic_json_visitor2() = default;

        virtual ~basic_json_visitor2() noexcept = default;

        void flush()
        {
            visit_flush();
        }

        bool begin_object(semantic_tag tag=semantic_tag::none,
                          const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_object(tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_object(std::size_t length, 
                          semantic_tag tag=semantic_tag::none, 
                          const ser_context& context = ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_object(length, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool end_object(const ser_context& context = ser_context())
        {
            std::error_code ec;
            bool more = visit_end_object(context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_array(semantic_tag tag=semantic_tag::none,
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_array(tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_array(std::size_t length, 
                         semantic_tag tag=semantic_tag::none,
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_begin_array(length, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool end_array(const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_end_array(context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool key(const string_view_type& name, const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_string(name, semantic_tag::none, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool null_value(semantic_tag tag = semantic_tag::none,
                        const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_null(tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool bool_value(bool value, 
                        semantic_tag tag = semantic_tag::none,
                        const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_bool(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool string_value(const string_view_type& value, 
                          semantic_tag tag = semantic_tag::none, 
                          const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_string(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        template <class Source>
        bool byte_string_value(const Source& b, 
                               semantic_tag tag=semantic_tag::none, 
                               const ser_context& context=ser_context(),
                               typename std::enable_if<type_traits::is_byte_sequence<Source>::value,int>::type = 0)
        {
            std::error_code ec;
            bool more = visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        template <class Source>
        bool byte_string_value(const Source& b, 
                               uint64_t ext_tag, 
                               const ser_context& context=ser_context(),
                               typename std::enable_if<type_traits::is_byte_sequence<Source>::value,int>::type = 0)
        {
            std::error_code ec;
            bool more = visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), ext_tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool uint64_value(uint64_t value, 
                          semantic_tag tag = semantic_tag::none, 
                          const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_uint64(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool int64_value(int64_t value, 
                         semantic_tag tag = semantic_tag::none, 
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_int64(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool half_value(uint16_t value, 
                        semantic_tag tag = semantic_tag::none, 
                        const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_half(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool double_value(double value, 
                          semantic_tag tag = semantic_tag::none, 
                          const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_double(value, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_object(semantic_tag tag,
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_begin_object(tag, context, ec);
        }

        bool begin_object(std::size_t length, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_begin_object(length, tag, context, ec);
        }

        bool end_object(const ser_context& context, std::error_code& ec)
        {
            return visit_end_object(context, ec);
        }

        bool begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec)
        {
            return visit_begin_array(tag, context, ec);
        }

        bool begin_array(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec)
        {
            return visit_begin_array(length, tag, context, ec);
        }

        bool end_array(const ser_context& context, std::error_code& ec)
        {
            return visit_end_array(context, ec);
        }

        bool key(const string_view_type& name, const ser_context& context, std::error_code& ec)
        {
            return visit_string(name, semantic_tag::none, context, ec);
        }

        bool null_value(semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) 
        {
            return visit_null(tag, context, ec);
        }

        bool bool_value(bool value, 
                        semantic_tag tag,
                        const ser_context& context,
                        std::error_code& ec) 
        {
            return visit_bool(value, tag, context, ec);
        }

        bool string_value(const string_view_type& value, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec) 
        {
            return visit_string(value, tag, context, ec);
        }

        template <class Source>
        bool byte_string_value(const Source& b, 
                               semantic_tag tag, 
                               const ser_context& context,
                               std::error_code& ec,
                               typename std::enable_if<type_traits::is_byte_sequence<Source>::value,int>::type = 0)
        {
            return visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), tag, context, ec);
        }

        template <class Source>
        bool byte_string_value(const Source& b, 
                               uint64_t ext_tag, 
                               const ser_context& context,
                               std::error_code& ec,
                               typename std::enable_if<type_traits::is_byte_sequence<Source>::value,int>::type = 0)
        {
            return visit_byte_string(byte_string_view(reinterpret_cast<const uint8_t*>(b.data()),b.size()), ext_tag, context, ec);
        }

        bool uint64_value(uint64_t value, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_uint64(value, tag, context, ec);
        }

        bool int64_value(int64_t value, 
                         semantic_tag tag, 
                         const ser_context& context,
                         std::error_code& ec)
        {
            return visit_int64(value, tag, context, ec);
        }

        bool half_value(uint16_t value, 
                        semantic_tag tag, 
                        const ser_context& context,
                        std::error_code& ec)
        {
            return visit_half(value, tag, context, ec);
        }

        bool double_value(double value, 
                          semantic_tag tag, 
                          const ser_context& context,
                          std::error_code& ec)
        {
            return visit_double(value, tag, context, ec);
        }

        template <class T>
        bool typed_array(const jsoncons::span<T>& data, 
                         semantic_tag tag=semantic_tag::none,
                         const ser_context& context=ser_context())
        {
            std::error_code ec;
            bool more = visit_typed_array(data, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        template <class T>
        bool typed_array(const jsoncons::span<T>& data, 
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code& ec)
        {
            return visit_typed_array(data, tag, context, ec);
        }

        bool typed_array(half_arg_t, const jsoncons::span<const uint16_t>& s,
            semantic_tag tag = semantic_tag::none,
            const ser_context& context = ser_context())
        {
            std::error_code ec;
            bool more = visit_typed_array(half_arg, s, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool typed_array(half_arg_t, const jsoncons::span<const uint16_t>& s,
                         semantic_tag tag,
                         const ser_context& context,
                         std::error_code& ec)
        {
            return visit_typed_array(half_arg, s, tag, context, ec);
        }

        bool begin_multi_dim(const jsoncons::span<const size_t>& shape,
                             semantic_tag tag = semantic_tag::multi_dim_row_major,
                             const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_begin_multi_dim(shape, tag, context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool begin_multi_dim(const jsoncons::span<const size_t>& shape,
                             semantic_tag tag,
                             const ser_context& context,
                             std::error_code& ec) 
        {
            return visit_begin_multi_dim(shape, tag, context, ec);
        }

        bool end_multi_dim(const ser_context& context=ser_context()) 
        {
            std::error_code ec;
            bool more = visit_end_multi_dim(context, ec);
            if (ec)
            {
                JSONCONS_THROW(ser_error(ec, context.line(), context.column()));
            }
            return more;
        }

        bool end_multi_dim(const ser_context& context,
                           std::error_code& ec) 
        {
            return visit_end_multi_dim(context, ec);
        }

    private:

        virtual void visit_flush() = 0;

        virtual bool visit_begin_object(semantic_tag tag, 
                                        const ser_context& context, 
                                        std::error_code& ec) = 0;

        virtual bool visit_begin_object(std::size_t /*length*/, 
                                        semantic_tag tag, 
                                        const ser_context& context, 
                                        std::error_code& ec)
        {
            return visit_begin_object(tag, context, ec);
        }

        virtual bool visit_end_object(const ser_context& context, 
                                      std::error_code& ec) = 0;

        virtual bool visit_begin_array(semantic_tag tag, 
                                       const ser_context& context, 
                                       std::error_code& ec) = 0;

        virtual bool visit_begin_array(std::size_t /*length*/, 
                                       semantic_tag tag, 
                                       const ser_context& context, 
                                       std::error_code& ec)
        {
            return visit_begin_array(tag, context, ec);
        }

        virtual bool visit_end_array(const ser_context& context, 
                                     std::error_code& ec) = 0;

        virtual bool visit_null(semantic_tag tag, 
                             const ser_context& context, 
                             std::error_code& ec) = 0;

        virtual bool visit_bool(bool value, 
                             semantic_tag tag, 
                             const ser_context& context, 
                             std::error_code&) = 0;

        virtual bool visit_string(const string_view_type& value, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) = 0;

        virtual bool visit_byte_string(const byte_string_view& value, 
                                    semantic_tag tag, 
                                    const ser_context& context,
                                    std::error_code& ec) = 0;

        virtual bool visit_byte_string(const byte_string_view& value, 
                                       uint64_t /*ext_tag*/, 
                                       const ser_context& context,
                                       std::error_code& ec) 
        {
            return visit_byte_string(value, semantic_tag::none, context, ec);
        }

        virtual bool visit_uint64(uint64_t value, 
                               semantic_tag tag, 
                               const ser_context& context,
                               std::error_code& ec) = 0;

        virtual bool visit_int64(int64_t value, 
                              semantic_tag tag,
                              const ser_context& context,
                              std::error_code& ec) = 0;

        virtual bool visit_half(uint16_t value, 
                             semantic_tag tag,
                             const ser_context& context,
                             std::error_code& ec)
        {
            return visit_double(binary::decode_half(value),
                             tag,
                             context,
                             ec);
        }

        virtual bool visit_double(double value, 
                               semantic_tag tag,
                               const ser_context& context,
                               std::error_code& ec) = 0;

        virtual bool visit_typed_array(const jsoncons::span<const uint8_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const uint32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) 
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const uint64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) 
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = uint64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const int8_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const int16_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const int32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const int64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = int64_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(half_arg_t, 
                                    const jsoncons::span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag, context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = half_value(*p, semantic_tag::none, context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const float>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = double_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_typed_array(const jsoncons::span<const double>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec)  
        {
            bool more = begin_array(s.size(), tag,context, ec);
            for (auto p = s.begin(); more && p != s.end(); ++p)
            {
                more = double_value(*p,semantic_tag::none,context, ec);
            }
            if (more)
            {
                more = end_array(context, ec);
            }
            return more;
        }

        virtual bool visit_begin_multi_dim(const jsoncons::span<const size_t>& shape,
                                        semantic_tag tag,
                                        const ser_context& context, 
                                        std::error_code& ec) 
        {
            bool more = visit_begin_array(2, tag, context, ec);
            if (more)
            {
                more = visit_begin_array(shape.size(), tag, context, ec);
                for (auto it = shape.begin(); more && it != shape.end(); ++it)
                {
                    visit_uint64(*it, semantic_tag::none, context, ec);
                }
                if (more)
                {
                    more = visit_end_array(context, ec);
                }
            }
            return more;
        }

        virtual bool visit_end_multi_dim(const ser_context& context,
                                      std::error_code& ec) 
        {
            return visit_end_array(context, ec);
        }
    };

 template <class CharT, class Allocator>
    class basic_json_visitor2_to_visitor_adaptor : public basic_json_visitor2<CharT>
    {
    public:
        using typename basic_json_visitor2<CharT>::char_type;
        using typename basic_json_visitor2<CharT>::string_view_type;
    private:
        using char_allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<char_type>;

        using string_type = std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type>;

        enum class container_t {root, array, object};
        enum class target_t {destination, buffer};

        struct level
        {
        private:
            target_t state_;
            container_t type_;
            int even_odd_;
            std::size_t count_;
        public:

            level(target_t state, container_t type) noexcept
                : state_(state), type_(type), even_odd_(type == container_t::object ? 0 : 1), count_(0)
            {
            }

            void advance()
            {
                if (!is_key())
                {
                    ++count_;
                }
                if (is_object())
                {
                    even_odd_ = !even_odd_;
                }
            }

            bool is_key() const
            {
                return even_odd_ == 0;
            }

            bool is_object() const
            {
                return type_ == container_t::object;
            }

            target_t target() const
            {
                return state_;
            }

            std::size_t count() const
            {
                return count_;
            }
        };
        using level_allocator_type = typename std::allocator_traits<Allocator>:: template rebind_alloc<level>;

        basic_default_json_visitor<char_type> default_visitor_;
        basic_json_visitor<char_type>* destination_;
        string_type key_;
        string_type key_buffer_;
        std::vector<level,level_allocator_type> level_stack_;

        const std::basic_string<char> null_k = {'n','u','l','l'};
        const std::basic_string<char> true_k = { 't','r','u','e' };
        const std::basic_string<char> false_k = { 'f', 'a', 'l', 's', 'e' };

        // noncopyable and nonmoveable
        basic_json_visitor2_to_visitor_adaptor(const basic_json_visitor2_to_visitor_adaptor&) = delete;
        basic_json_visitor2_to_visitor_adaptor& operator=(const basic_json_visitor2_to_visitor_adaptor&) = delete;
    public:
        explicit basic_json_visitor2_to_visitor_adaptor(const Allocator& alloc = Allocator())
            : default_visitor_(), destination_(std::addressof(default_visitor_)),
              key_(alloc), key_buffer_(alloc), level_stack_(alloc)
        {
            level_stack_.emplace_back(target_t::destination,container_t::root); // root
        }

        explicit basic_json_visitor2_to_visitor_adaptor(basic_json_visitor<char_type>& visitor, 
                                                     const Allocator& alloc = Allocator())
            : destination_(std::addressof(visitor)), 
              key_(alloc), key_buffer_(alloc), level_stack_(alloc)
        {
            level_stack_.emplace_back(target_t::destination,container_t::root); // root
        }

        basic_json_visitor<char_type>& destination()
        {
            return *destination_;
        }

        void destination(basic_json_visitor<char_type>& dest)
        {
            destination_ = std::addressof(dest);
        }

    private:
        void visit_flush() override
        {
            destination_->flush();
        }

        bool visit_begin_object(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            if (level_stack_.back().is_key())
            {
                if (level_stack_.back().target() == target_t::buffer && level_stack_.back().count() > 0)
                {
                    key_buffer_.push_back(',');
                }
                level_stack_.emplace_back(target_t::buffer, container_t::object);
                key_buffer_.push_back('{');
                return true;
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        level_stack_.emplace_back(target_t::buffer, container_t::object);
                        key_buffer_.push_back('{');
                        return true;
                    default:
                        level_stack_.emplace_back(target_t::destination, container_t::object);
                        return destination_->begin_object(tag, context, ec);
                }
            }
        }

        bool visit_begin_object(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            if (level_stack_.back().is_key())
            {
                if (level_stack_.back().target() == target_t::buffer && level_stack_.back().count() > 0)
                {
                    key_buffer_.push_back(',');
                }
                level_stack_.emplace_back(target_t::buffer, container_t::object);
                key_buffer_.push_back('{');
                return true;
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        level_stack_.emplace_back(target_t::buffer, container_t::object);
                        key_buffer_.push_back('{');
                        return true;
                    default:
                        level_stack_.emplace_back(target_t::destination, container_t::object);
                        return destination_->begin_object(length, tag, context, ec);
                }
            }
        }

        bool visit_end_object(const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;
            switch (level_stack_.back().target())
            {
                case target_t::buffer:
                    key_buffer_.push_back('}');
                    JSONCONS_ASSERT(level_stack_.size() > 1);
                    level_stack_.pop_back();
                    
                    if (level_stack_.back().target() == target_t::destination)
                    {
                        retval = destination_->key(key_buffer_,context, ec);
                        key_buffer_.clear();
                    }
                    else if (level_stack_.back().is_key())
                    {
                        key_buffer_.push_back(':');
                    }
                    level_stack_.back().advance();
                    break;
                default:
                    JSONCONS_ASSERT(level_stack_.size() > 1);
                    level_stack_.pop_back();
                    level_stack_.back().advance();
                    retval = destination_->end_object(context, ec);
                    break;
            }
            return retval;
        }

        bool visit_begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            if (level_stack_.back().is_key())
            {
                if (level_stack_.back().target() == target_t::buffer && level_stack_.back().count() > 0)
                {
                    key_buffer_.push_back(',');
                }
                level_stack_.emplace_back(target_t::buffer, container_t::array);
                key_buffer_.push_back('[');
                return true;
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        level_stack_.emplace_back(target_t::buffer, container_t::array);
                        key_buffer_.push_back('[');
                        return true;
                    default:
                        level_stack_.emplace_back(target_t::destination, container_t::array);
                        return destination_->begin_array(tag, context, ec);
                }
            }
        }

        bool visit_begin_array(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            if (level_stack_.back().is_key())
            {
                if (level_stack_.back().target() == target_t::buffer && level_stack_.back().count() > 0)
                {
                    key_buffer_.push_back(',');
                }
                level_stack_.emplace_back(target_t::buffer, container_t::array);
                key_buffer_.push_back('[');
                return true;
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        level_stack_.emplace_back(target_t::buffer, container_t::array);
                        key_buffer_.push_back('[');
                        return true;
                    default:
                        level_stack_.emplace_back(target_t::destination, container_t::array);
                        return destination_->begin_array(length, tag, context, ec);
                }
            }
        }

        bool visit_end_array(const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;
            switch (level_stack_.back().target())
            {
                case target_t::buffer:
                    key_buffer_.push_back(']');
                    JSONCONS_ASSERT(level_stack_.size() > 1);
                    level_stack_.pop_back();
                    if (level_stack_.back().target() == target_t::destination)
                    {
                        retval = destination_->key(key_buffer_, context, ec);
                        key_buffer_.clear();
                    }
                    else if (level_stack_.back().is_key())
                    {
                        key_buffer_.push_back(':');
                    }
                    level_stack_.back().advance();
                    break;
                default:
                    JSONCONS_ASSERT(level_stack_.size() > 1);
                    level_stack_.pop_back();
                    level_stack_.back().advance();
                    retval = destination_->end_array(context, ec);
                    break;
            }
            return retval;
        }

        bool visit_string(const string_view_type& value,
                             semantic_tag tag,
                             const ser_context& context,
                             std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), value.begin(), value.end());
                        key_buffer_.push_back('\"');
                        key_buffer_.push_back(':');
                        retval = true;
                        break;
                    default:
                        retval = destination_->key(value, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), value.begin(), value.end());
                        key_buffer_.push_back('\"');
                        retval = true;
                        break;
                    default:
                        retval = destination_->string_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_byte_string(const byte_string_view& value, 
                                  semantic_tag tag,
                                  const ser_context& context,
                                  std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_.clear();
                switch (tag)
                {
                    case semantic_tag::base64:
                        encode_base64(value.begin(), value.end(), key_);
                        break;
                    case semantic_tag::base16:
                        encode_base16(value.begin(), value.end(),key_);
                        break;
                    default:
                        encode_base64url(value.begin(), value.end(),key_);
                        break;
                }
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back('\"');
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back('\"');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->byte_string_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_byte_string(const byte_string_view& value, 
                               uint64_t ext_tag,
                               const ser_context& context,
                               std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_.clear();
                encode_base64url(value.begin(), value.end(),key_);
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back('\"');
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.push_back('\"');
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back('\"');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->byte_string_value(value, ext_tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_uint64(uint64_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_.clear();
                jsoncons::detail::from_integer(value,key_);
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        retval = true; 
                        break;
                    default:
                        retval = destination_->uint64_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_int64(int64_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_.clear();
                jsoncons::detail::from_integer(value,key_);
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        retval = true; 
                        break;
                    default:
                        retval = destination_->int64_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_half(uint16_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_.clear();
                jsoncons::string_sink<string_type> sink(key_);
                jsoncons::detail::write_double f{float_chars_format::general,0};
                double x = binary::decode_half(value);
                f(x, sink);
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        retval = true; 
                        break;
                    default:
                        retval = destination_->half_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_double(double value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_.clear();
                string_sink<string_type> sink(key_);
                jsoncons::detail::write_double f{float_chars_format::general,0};
                f(value, sink);
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        retval = true; 
                        break;
                    default:
                        retval = destination_->double_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_bool(bool value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_ = value ? true_k : false_k;
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        retval = true; 
                        break;
                    default:
                        retval = destination_->bool_value(value, tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_null(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            bool retval = true;

            if (level_stack_.back().is_key() || level_stack_.back().target() == target_t::buffer)
            {
                key_ = null_k;
            }

            if (level_stack_.back().is_key())
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        key_buffer_.push_back(':');
                        retval = true; 
                        break;
                    default:
                        retval = destination_->key(key_, context, ec);
                        break;
                }
            }
            else
            {
                switch (level_stack_.back().target())
                {
                    case target_t::buffer:
                        if (!level_stack_.back().is_object() && level_stack_.back().count() > 0)
                        {
                            key_buffer_.push_back(',');
                        }
                        key_buffer_.insert(key_buffer_.end(), key_.begin(), key_.end());
                        retval = true; 
                        break;
                    default:
                        retval = destination_->null_value(tag, context, ec);
                        break;
                }
            }

            level_stack_.back().advance();
            return retval;
        }

        bool visit_typed_array(const jsoncons::span<const uint8_t>& s, 
                               semantic_tag tag,
                               const ser_context& context, 
                               std::error_code& ec) override 
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const uint32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override 
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const uint64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override 
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const int8_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const int16_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const int32_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const int64_t>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(half_arg_t, 
                               const jsoncons::span<const uint16_t>& s, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(half_arg,s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(half_arg, s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const float>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }

        bool visit_typed_array(const jsoncons::span<const double>& s, 
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) override  
        {
            bool is_key = level_stack_.back().is_key();
            level_stack_.back().advance();

            if (is_key || level_stack_.back().target() == target_t::buffer)
            {
                return basic_json_visitor2<CharT>::visit_typed_array(s,tag,context,ec);
            }
            else
            {
                return destination_->typed_array(s, tag, context, ec);
            }
        }
    };

    template <class CharT>
    class basic_default_json_visitor2 : public basic_json_visitor2<CharT>
    {
        bool parse_more_;
        std::error_code ec_;
    public:
        using typename basic_json_visitor2<CharT>::string_view_type;

        basic_default_json_visitor2(bool accept_more = true,
                                    std::error_code ec = std::error_code())
            : parse_more_(accept_more), ec_(ec)
        {
        }
    private:
        void visit_flush() override
        {
        }

        bool visit_begin_object(semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_begin_object(std::size_t, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_end_object(const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_begin_array(semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_begin_array(std::size_t, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_end_array(const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_null(semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_string(const string_view_type&, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_byte_string(const byte_string_view&, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_uint64(uint64_t, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_int64(int64_t, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_half(uint16_t, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_double(double, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }

        bool visit_bool(bool, semantic_tag, const ser_context&, std::error_code& ec) override
        {
            if (ec_)
            {
                ec = ec_;
            }
            return parse_more_;
        }
    };

    // basic_json_visitor_to_visitor2_adaptor

    template <class CharT>
    class basic_json_visitor_to_visitor2_adaptor : public basic_json_visitor<CharT>
    {
    public:
        using typename basic_json_visitor<CharT>::char_type;
        using typename basic_json_visitor<CharT>::string_view_type;
    private:
        basic_json_visitor2<char_type>& destination_;

        // noncopyable and nonmoveable
        basic_json_visitor_to_visitor2_adaptor(const basic_json_visitor_to_visitor2_adaptor&) = delete;
        basic_json_visitor_to_visitor2_adaptor& operator=(const basic_json_visitor_to_visitor2_adaptor&) = delete;
    public:
        basic_json_visitor_to_visitor2_adaptor(basic_json_visitor2<char_type>& visitor)
            : destination_(visitor)
        {
        }

        basic_json_visitor2<char_type>& destination()
        {
            return destination_;
        }

    private:
        void visit_flush() override
        {
            destination_.flush();
        }

        bool visit_begin_object(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.begin_object(tag, context, ec);
        }

        bool visit_begin_object(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.begin_object(length, tag, context, ec);
        }

        bool visit_end_object(const ser_context& context, std::error_code& ec) override
        {
            return destination_.end_object(context, ec);
        }

        bool visit_begin_array(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.begin_array(tag, context, ec);
        }

        bool visit_begin_array(std::size_t length, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.begin_array(length, tag, context, ec);
        }

        bool visit_end_array(const ser_context& context, std::error_code& ec) override
        {
            return destination_.end_array(context, ec);
        }

        bool visit_key(const string_view_type& name,
                       const ser_context& context,
                       std::error_code& ec) override
        {
            return destination_.visit_string(name, context, ec);
        }

        bool visit_string(const string_view_type& value,
                          semantic_tag tag,
                          const ser_context& context,
                          std::error_code& ec) override
        {
            return destination_.string_value(value, tag, context, ec);
        }

        bool visit_byte_string(const byte_string_view& b, 
                               semantic_tag tag,
                               const ser_context& context,
                               std::error_code& ec) override
        {
            return destination_.byte_string_value(b, tag, context, ec);
        }

        bool visit_uint64(uint64_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.uint64_value(value, tag, context, ec);
        }

        bool visit_int64(int64_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.int64_value(value, tag, context, ec);
        }

        bool visit_half(uint16_t value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.half_value(value, tag, context, ec);
        }

        bool visit_double(double value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.double_value(value, tag, context, ec);
        }

        bool visit_bool(bool value, semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.bool_value(value, tag, context, ec);
        }

        bool visit_null(semantic_tag tag, const ser_context& context, std::error_code& ec) override
        {
            return destination_.null_value(tag, context, ec);
        }
    };

    class diagnostics_visitor2 : public basic_default_json_visitor2<char>
    {
        bool visit_begin_object(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_object" << std::endl; 
            return true;
        }

        bool visit_begin_object(std::size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_object " << length << std::endl; 
            return true;
        }

        bool visit_end_object(const ser_context&, std::error_code&) override
        {
            std::cout << "visit_end_object" << std::endl; 
            return true;
        }

        bool visit_begin_array(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_array" << std::endl;
            return true;
        }

        bool visit_begin_array(std::size_t length, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_begin_array " << length << std::endl; 
            return true;
        }

        bool visit_end_array(const ser_context&, std::error_code&) override
        {
            std::cout << "visit_end_array" << std::endl; 
            return true;
        }

        bool visit_string(const string_view_type& s, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_string " << s << std::endl; 
            return true;
        }
        bool visit_int64(int64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_int64 " << val << std::endl; 
            return true;
        }
        bool visit_uint64(uint64_t val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_uint64 " << val << std::endl; 
            return true;
        }
        bool visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_bool " << val << std::endl; 
            return true;
        }
        bool visit_null(semantic_tag, const ser_context&, std::error_code&) override
        {
            std::cout << "visit_null " << std::endl; 
            return true;
        }

        bool visit_typed_array(const jsoncons::span<const uint16_t>& s, 
                                    semantic_tag tag, 
                                    const ser_context&, 
                                    std::error_code&) override  
        {
            std::cout << "visit_typed_array uint16_t " << tag << std::endl; 
            for (auto val : s)
            {
                std::cout << val << "" << std::endl;
            }
            std::cout << "" << std::endl;
            return true;
        }

        bool visit_typed_array(half_arg_t, const jsoncons::span<const uint16_t>& s,
            semantic_tag tag,
            const ser_context&,
            std::error_code&) override
        {
            std::cout << "visit_typed_array half_arg_t uint16_t " << tag << "" << std::endl;
            for (auto val : s)
            {
                std::cout << val << "" << std::endl;
            }
            std::cout << "" << std::endl;
            return true;
        }
    };

    using json_visitor2 = basic_json_visitor2<char>;
    using default_json_visitor2 = basic_default_json_visitor2<char>;
    using json_visitor2_to_visitor_adaptor = basic_json_visitor2_to_visitor_adaptor<char>;

} // namespace jsoncons

#endif
