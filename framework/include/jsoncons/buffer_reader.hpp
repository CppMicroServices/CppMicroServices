// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BUFFER_READER_HPP
#define JSONCONS_BUFFER_READER_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <memory> // std::allocator_traits
#include <vector> // std::vector
#include <jsoncons/unicode_traits.hpp>
#include <jsoncons/json_error.hpp> // json_errc
#include <jsoncons/source.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons {

    // buffer_reader

    template<class CharT,class Allocator=std::allocator<char>>
    class buffer_reader 
    {
    public:
        using char_type = CharT;
    private:
        typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<char_type> char_allocator_type;

        std::vector<char_type,char_allocator_type> buffer_;
        const char_type* data_;
        std::size_t length_;
        bool bof_;
        bool eof_;

    public:

        buffer_reader(std::size_t buffer_length, const Allocator& alloc = Allocator())
            : buffer_(buffer_length, CharT(), alloc), data_(nullptr), length_(0), bof_(true), eof_(false)
        {
        }

        bool eof() const
        {
            return eof_;
        }

        std::size_t buffer_length() const
        {
            return buffer_.size();
        }

        void buffer_length(std::size_t length)
        {
            buffer_.resize(length);
        }

        const char_type* data() const {return data_;}
        std::size_t length() const {return length_;}

        template <class Source>
        void read(Source& source, std::error_code& ec)
        {
            if (!eof_)
            {
                if (source.eof())
                {
                    eof_ = true;
                }
                else
                {
                    data_ = buffer_.data();
                    length_ = source.read(buffer_.data(), buffer_.size());

                    if (buffer_.empty())
                    {
                        eof_ = true;
                    }
                    else if (bof_)
                    {
                        auto r = unicode_traits::detect_encoding_from_bom(buffer_.data(), length_);
                        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
                        {
                            ec = json_errc::illegal_unicode_character;
                            return;
                        }
                        length_ -= (r.ptr - data_);
                        data_ = r.ptr;
                        bof_ = false;
                    }
                }
            }
        }
    };

    // json_buffer_reader

    template<class CharT,class Allocator>
    class json_buffer_reader 
    {
    public:
        using char_type = CharT;
    private:
        typedef typename std::allocator_traits<Allocator>:: template rebind_alloc<char_type> char_allocator_type;

        std::vector<char_type,char_allocator_type> buffer_;
        const char_type* data_;
        std::size_t length_;
        bool bof_;
        bool eof_;

    public:

        json_buffer_reader(std::size_t buffer_length, const Allocator& alloc)
            : buffer_(buffer_length, CharT(), alloc), data_(nullptr), length_(0), bof_(true), eof_(false)
        {
        }

        bool eof() const
        {
            return eof_;
        }

        std::size_t buffer_length() const
        {
            return buffer_.size();
        }

        void buffer_length(std::size_t length)
        {
            buffer_.resize(length);
        }

        const char_type* data() const {return data_;}
        std::size_t length() const {return length_;}

        template <class Source>
        void read(Source& source, std::error_code& ec)
        {
            if (!eof_)
            {
                if (source.eof())
                {
                    eof_ = true;
                }
                else
                {
                    data_ = buffer_.data();
                    length_ = source.read(buffer_.data(), buffer_.size());

                    if (buffer_.empty())
                    {
                        eof_ = true;
                    }
                    else if (bof_)
                    {
                        auto r = unicode_traits::detect_json_encoding(buffer_.data(), length_);
                        if (!(r.encoding == unicode_traits::encoding_kind::utf8 || r.encoding == unicode_traits::encoding_kind::undetected))
                        {
                            ec = json_errc::illegal_unicode_character;
                            return;
                        }
                        length_ -= (r.ptr - data_);
                        data_ = r.ptr;
                        bof_ = false;
                    }
                }
            }
        }
    };

} // namespace jsoncons

#endif

