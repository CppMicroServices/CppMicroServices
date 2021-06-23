// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONVERTER_HPP
#define JSONCONS_CONVERTER_HPP

#include <system_error> // std::error_code
#include <jsoncons/more_type_traits.hpp>
#include <jsoncons/byte_string.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/conv_error.hpp>
#include <jsoncons/detail/write_number.hpp> // from_integer

namespace jsoncons {

    template <class Into, class Enable=void>
    class converter
    {
    };

    // Into list like of bytes
    template <class Into>
    class converter<Into,typename std::enable_if<(!type_traits::is_basic_string<Into>::value && 
                                                   type_traits::is_back_insertable_byte_container<Into>::value) ||
                                                   type_traits::is_basic_byte_string<Into>::value>::type>
    {
        using value_type = typename Into::value_type;
        using allocator_type = typename Into::allocator_type;

    public:

        JSONCONS_CPP14_CONSTEXPR 
        Into from(const byte_string_view& bstr, semantic_tag tag, std::error_code& ec) const
        {
            Into bytes;
            from_(bytes, bstr, tag, ec);
            return bytes;
        }

        JSONCONS_CPP14_CONSTEXPR 
        Into from(const byte_string_view& bstr, semantic_tag tag, const allocator_type& alloc, std::error_code& ec) const
        {
            Into bytes(alloc);
            from_(bytes, bstr, tag, ec);
            return bytes;
        }

        template <class CharT>
        JSONCONS_CPP14_CONSTEXPR 
        Into from(const jsoncons::basic_string_view<CharT>& s, semantic_tag tag, std::error_code& ec) const
        {
            Into bytes;
            from_(bytes, s, tag, ec);
            return bytes;
        }

        template <class CharT>
        JSONCONS_CPP14_CONSTEXPR 
        Into from(const jsoncons::basic_string_view<CharT>& s, semantic_tag tag, const allocator_type& alloc, std::error_code& ec) const
        {
            Into bytes(alloc);
            from_(bytes, s, tag, ec);
            return bytes;
        }

    private:
        JSONCONS_CPP14_CONSTEXPR 
        void from_(Into& bytes, const byte_string_view& bstr, semantic_tag, std::error_code&) const
        {
            for (auto ch : bstr)
            {
                bytes.push_back(static_cast<value_type>(ch));
            }
        }

        template <class CharT>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_narrow_character<CharT>::value>::type
        from_(Into& bytes, const jsoncons::basic_string_view<CharT>& s, semantic_tag tag, std::error_code& ec) const
        {
            switch (tag)
            {
                case semantic_tag::base16:
                {
                    auto res = decode_base16(s.begin(), s.end(), bytes);
                    if (res.ec != conv_errc::success)
                    {
                        ec = conv_errc::not_byte_string;
                    }
                    break;
                }
                case semantic_tag::base64:
                {
                    decode_base64(s.begin(), s.end(), bytes);
                    break;
                }
                case semantic_tag::base64url:
                {
                    decode_base64url(s.begin(), s.end(), bytes);
                    break;
                }
                default:
                {
                    ec = conv_errc::not_byte_string;
                    break;
                }
            }
        }

        template <class CharT>
        typename std::enable_if<type_traits::is_wide_character<CharT>::value>::type
        from_(Into& bytes, const jsoncons::basic_string_view<CharT>& s, semantic_tag tag, std::error_code& ec) const
        {
            std::string u;
            auto retval = unicode_traits::convert(s.data(), s.size(), u);
            if (retval.ec != unicode_traits::conv_errc())
            {
                ec = conv_errc::not_utf8;
                return;
            }
            from_(bytes, jsoncons::string_view(u), tag, ec);
        }
    };

    // Into string
    template <class Into>
    class converter<Into,typename std::enable_if<type_traits::is_basic_string<Into>::value>::type>
    {
        using char_type = typename Into::value_type;
        using allocator_type = typename Into::allocator_type;
        int dummy_;
    public:
        explicit converter(int dummy = int())
            : dummy_(dummy)
        {}
        template<class Integer>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_integer<Integer>::value,Into>::type
        from(Integer val, semantic_tag, std::error_code&) const
        {
            Into s;
            jsoncons::detail::from_integer(val, s);
            return s;
        }

        template<class Integer>
        JSONCONS_CPP14_CONSTEXPR 
        typename std::enable_if<type_traits::is_integer<Integer>::value,Into>::type
        from(Integer val, semantic_tag, const allocator_type& alloc, std::error_code&) const
        {
            Into s(alloc);
            jsoncons::detail::from_integer(val, s);
            return s;
        }

        Into from(double val, semantic_tag, std::error_code&) const
        {
            Into s;
            jsoncons::detail::write_double f{float_chars_format::general,0};
            f(val, s);
            return s;
        }

        Into from(double val, semantic_tag, const allocator_type& alloc, std::error_code&) const
        {
            Into s(alloc);
            jsoncons::detail::write_double f{float_chars_format::general,0};
            f(val, s);
            return s;
        }

        Into from(half_arg_t, uint16_t val, semantic_tag, std::error_code&) const
        {
            Into s;
            jsoncons::detail::write_double f{float_chars_format::general,0};
            double x = binary::decode_half(val);
            f(x, s);
            return s;
        }

        Into from(half_arg_t, uint16_t val, semantic_tag, const allocator_type& alloc, std::error_code&) const
        {
            Into s(alloc);
            jsoncons::detail::write_double f{float_chars_format::general,0};
            double x = binary::decode_half(val);
            f(x, s);
            return s;
        }

        template <class ChT = char_type>
        JSONCONS_CPP14_CONSTEXPR
        Into from(const byte_string_view& bytes, semantic_tag tag, std::error_code& ec) const
        {
            Into s;
            from_(s, bytes, tag, ec);
            return s;
        }

        template <class ChT = char_type>
        JSONCONS_CPP14_CONSTEXPR
        Into from(const byte_string_view& bytes, semantic_tag tag, const allocator_type& alloc, std::error_code& ec) const
        {
            Into s(alloc);
            from_(s, bytes, tag, ec);
            return s;
        }

        constexpr 
        Into from(const jsoncons::basic_string_view<char_type>& s, semantic_tag, std::error_code&) const
        {
            return Into(s.data(), s.size());
        }

        constexpr 
        Into from(const jsoncons::basic_string_view<char_type>& s, semantic_tag, const allocator_type& alloc, std::error_code&) const
        {
            return Into(s.data(), s.size(), alloc);
        }

        JSONCONS_CPP14_CONSTEXPR 
        Into from(bool val, semantic_tag, std::error_code&) const
        {
            constexpr char_type true_literal[] = {'t','r','u','e'}; 
            constexpr char_type false_literal[] = {'f','a','l','s','e'}; 

            return val ? Into(true_literal,4) : Into(false_literal,5);
        }

        JSONCONS_CPP14_CONSTEXPR 
        Into from(bool val, semantic_tag, const allocator_type& alloc, std::error_code&) const
        {
            constexpr char_type true_literal[] = {'t','r','u','e'}; 
            constexpr char_type false_literal[] = {'f','a','l','s','e'}; 

            return val ? Into(true_literal,4,alloc) : Into(false_literal,5,alloc);
        }

        JSONCONS_CPP14_CONSTEXPR 
        Into from(null_type, semantic_tag, std::error_code&) const
        {
            constexpr char_type null_literal[] = {'n','u','l','l'}; 

            return Into(null_literal,4);
        }

        JSONCONS_CPP14_CONSTEXPR 
        Into from(null_type, semantic_tag, const allocator_type& alloc, std::error_code&) const
        {
            constexpr char_type null_literal[] = {'n','u','l','l'}; 

            return Into(null_literal,4,alloc);
        }
    private:

        template <class ChT = char_type>
        JSONCONS_CPP14_CONSTEXPR
        typename std::enable_if<type_traits::is_byte<ChT>::value>::type
        from_(Into& s, const byte_string_view& bytes, semantic_tag tag, std::error_code&) const
        {
            switch (tag)
            {
                case semantic_tag::base64:
                    encode_base64(bytes.begin(), bytes.end(), s);
                    break;
                case semantic_tag::base16:
                    encode_base16(bytes.begin(), bytes.end(), s);
                    break;
                default:
                    encode_base64url(bytes.begin(), bytes.end(), s);
                    break;
            }
        }

        template <class ChT = char_type>
        typename std::enable_if<!type_traits::is_byte<ChT>::value>::type
        from_(Into& s, const byte_string_view& bytes, semantic_tag tag, std::error_code& ec) const
        {
            converter<std::string> convert{ dummy_ };
            std::string u = convert.from(bytes, tag, ec);

            auto retval = unicode_traits::convert(u.data(), u.size(), s);
            if (retval.ec != unicode_traits::conv_errc())
            {
                ec = conv_errc::not_wide_char;
            }
        }

    };

} // namespace jsoncons

#endif

