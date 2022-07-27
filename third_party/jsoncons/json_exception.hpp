// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSON_EXCEPTION_HPP
#define JSON_EXCEPTION_HPP

#include <string> // std::string
#include <sstream> // std::ostringstream
#include <system_error> // std::error_code
#include <jsoncons/unicode_traits.hpp> // unicode_traits::convert
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/more_type_traits.hpp>

namespace jsoncons {

    // json_exception

    class json_exception
    {
    public:
        virtual ~json_exception() noexcept = default;
        virtual const char* what() const noexcept = 0;
    };

    // json_runtime_error

    template <class Base, class Enable = void>
    class json_runtime_error
    {
    };

    template <class Base>
    class json_runtime_error<Base,
                             typename std::enable_if<std::is_convertible<Base*,std::exception*>::value &&
                                                     type_traits::is_constructible_from_string<Base>::value>::type> 
        : public Base, public virtual json_exception
    {
    public:
        json_runtime_error(const std::string& s) noexcept
            : Base(s)
        {
        }
        ~json_runtime_error() noexcept
        {
        }
        const char* what() const noexcept override
        {
            return Base::what();
        }
    };

    class key_not_found : public std::out_of_range, public virtual json_exception
    {
        std::string name_;
        mutable std::string what_;
    public:
        template <class CharT>
        explicit key_not_found(const CharT* key, std::size_t length) noexcept
            : std::out_of_range("Key not found")
        {
            JSONCONS_TRY
            {
                unicode_traits::convert(key, length, name_,
                                 unicode_traits::conv_flags::strict);
            }
            JSONCONS_CATCH(...)
            {
            }
        }

        virtual ~key_not_found() noexcept
        {
        }

        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS_TRY
                {
                    what_.append(std::out_of_range::what());
                    what_.append(": '");
                    what_.append(name_);
                    what_.append("'");
                    return what_.c_str();
                }
                JSONCONS_CATCH(...)
                {
                    return std::out_of_range::what();
                }
            }
            else
            {
                return what_.c_str();
            }
        }
    };

    class not_an_object : public std::runtime_error, public virtual json_exception
    {
        std::string name_;
        mutable std::string what_;
    public:
        template <class CharT>
        explicit not_an_object(const CharT* key, std::size_t length) noexcept
            : std::runtime_error("Attempting to access a member of a value that is not an object")
        {
            JSONCONS_TRY
            {
                unicode_traits::convert(key, length, name_,
                                 unicode_traits::conv_flags::strict);
            }
            JSONCONS_CATCH(...)
            {
            }
        }

        virtual ~not_an_object() noexcept
        {
        }
        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS_TRY
                {
                    what_.append(std::runtime_error::what());
                    what_.append(": '");
                    what_.append(name_);
                    what_.append("'");
                    return what_.c_str();
                }
                JSONCONS_CATCH(...)
                {
                    return std::runtime_error::what();
                }
            }
            else
            {
                return what_.c_str();
            }
        }
    };

    class ser_error : public std::system_error, public virtual json_exception
    {
        std::size_t line_number_;
        std::size_t column_number_;
        mutable std::string what_;
    public:
        ser_error(std::error_code ec)
            : std::system_error(ec), line_number_(0), column_number_(0)
        {
        }
        ser_error(std::error_code ec, const std::string& what_arg)
            : std::system_error(ec, what_arg), line_number_(0), column_number_(0)
        {
        }
        ser_error(std::error_code ec, std::size_t position)
            : std::system_error(ec), line_number_(0), column_number_(position)
        {
        }
        ser_error(std::error_code ec, std::size_t line, std::size_t column)
            : std::system_error(ec), line_number_(line), column_number_(column)
        {
        }
        ser_error(const ser_error& other) = default;

        ser_error(ser_error&& other) = default;

        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS_TRY
                {
                    what_.append(std::system_error::what());
                    if (line_number_ != 0 && column_number_ != 0)
                    {
                        what_.append(" at line ");
                        what_.append(std::to_string(line_number_));
                        what_.append(" and column ");
                        what_.append(std::to_string(column_number_));
                    }
                    else if (column_number_ != 0)
                    {
                        what_.append(" at position ");
                        what_.append(std::to_string(column_number_));
                    }
                    return what_.c_str();
                }
                JSONCONS_CATCH(...)
                {
                    return std::system_error::what();
                }
            }
            else
            {
                return what_.c_str();
            }
        }

        std::size_t line() const noexcept
        {
            return line_number_;
        }

        std::size_t column() const noexcept
        {
            return column_number_;
        }

    #if !defined(JSONCONS_NO_DEPRECATED)
        JSONCONS_DEPRECATED_MSG("Instead, use line()")
        std::size_t line_number() const noexcept
        {
            return line();
        }

        JSONCONS_DEPRECATED_MSG("Instead, use column()")
        std::size_t column_number() const noexcept
        {
            return column();
        }
    #endif
    };

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error serialization_error;
JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error json_parse_exception;
JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error parse_exception;
JSONCONS_DEPRECATED_MSG("Instead, use ser_error") typedef ser_error parse_error;
typedef ser_error codec_error;
#endif

} // namespace jsoncons

#endif
