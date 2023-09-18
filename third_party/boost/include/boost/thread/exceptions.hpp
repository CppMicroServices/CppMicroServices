// Copyright (C) 2001-2003
// William E. Kempf
// Copyright (C) 2007-9 Anthony Williams
// (C) Copyright 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_THREAD_EXCEPTIONS_PDM070801_H
#define BOOST_THREAD_EXCEPTIONS_PDM070801_H

#include <boost/thread/detail/config.hpp>

//  pdm: Sorry, but this class is used all over the place & I end up
//       with recursive headers if I don't separate it
//  wek: Not sure why recursive headers would cause compilation problems
//       given the include guards, but regardless it makes sense to
//       seperate this out any way.

#include <string>
#include <stdexcept>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>


#include <boost/config/abi_prefix.hpp>

namespace cppmsboost
{

#if defined BOOST_THREAD_PROVIDES_INTERRUPTIONS
    class BOOST_SYMBOL_VISIBLE thread_interrupted
    {};
#endif

    class BOOST_SYMBOL_VISIBLE thread_exception:
        public system::system_error
        //public std::exception
    {
          typedef system::system_error base_type;
    public:
        thread_exception()
          : base_type(0,system::generic_category())
        {}

        thread_exception(int sys_error_code)
          : base_type(sys_error_code, system::generic_category())
        {}

        thread_exception( int ev, const char * what_arg )
        : base_type(system::error_code(ev, system::generic_category()), what_arg)
        {
        }
        thread_exception( int ev, const std::string & what_arg )
        : base_type(system::error_code(ev, system::generic_category()), what_arg)
        {
        }

        ~thread_exception() BOOST_NOEXCEPT_OR_NOTHROW
        {}


        int native_error() const
        {
            return code().value();
        }

    };

    class BOOST_SYMBOL_VISIBLE condition_error:
        public system::system_error
        //public std::exception
    {
          typedef system::system_error base_type;
    public:
          condition_error()
          : base_type(system::error_code(0, system::generic_category()), "Condition error")
          {}
          condition_error( int ev )
          : base_type(system::error_code(ev, system::generic_category()), "Condition error")
          {
          }
          condition_error( int ev, const char * what_arg )
          : base_type(system::error_code(ev, system::generic_category()), what_arg)
          {
          }
          condition_error( int ev, const std::string & what_arg )
          : base_type(system::error_code(ev, system::generic_category()), what_arg)
          {
          }
    };


    class BOOST_SYMBOL_VISIBLE lock_error:
        public thread_exception
    {
          typedef thread_exception base_type;
    public:
        lock_error()
        : base_type(0, "cppmsboost::lock_error")
        {}

        lock_error( int ev )
        : base_type(ev, "cppmsboost::lock_error")
        {
        }
        lock_error( int ev, const char * what_arg )
        : base_type(ev, what_arg)
        {
        }
        lock_error( int ev, const std::string & what_arg )
        : base_type(ev, what_arg)
        {
        }

        ~lock_error() BOOST_NOEXCEPT_OR_NOTHROW
        {}

    };

    class BOOST_SYMBOL_VISIBLE thread_resource_error:
        public thread_exception
    {
          typedef thread_exception base_type;
    public:
          thread_resource_error()
          : base_type(static_cast<int>(system::errc::resource_unavailable_try_again), "cppmsboost::thread_resource_error")
          {}

          thread_resource_error( int ev )
          : base_type(ev, "cppmsboost::thread_resource_error")
          {
          }
          thread_resource_error( int ev, const char * what_arg )
          : base_type(ev, what_arg)
          {
          }
          thread_resource_error( int ev, const std::string & what_arg )
          : base_type(ev, what_arg)
          {
          }


        ~thread_resource_error() BOOST_NOEXCEPT_OR_NOTHROW
        {}

    };

    class BOOST_SYMBOL_VISIBLE unsupported_thread_option:
        public thread_exception
    {
          typedef thread_exception base_type;
    public:
          unsupported_thread_option()
          : base_type(static_cast<int>(system::errc::invalid_argument), "cppmsboost::unsupported_thread_option")
          {}

          unsupported_thread_option( int ev )
          : base_type(ev, "cppmsboost::unsupported_thread_option")
          {
          }
          unsupported_thread_option( int ev, const char * what_arg )
          : base_type(ev, what_arg)
          {
          }
          unsupported_thread_option( int ev, const std::string & what_arg )
          : base_type(ev, what_arg)
          {
          }

    };

    class BOOST_SYMBOL_VISIBLE invalid_thread_argument:
        public thread_exception
    {
          typedef thread_exception base_type;
    public:
        invalid_thread_argument()
        : base_type(static_cast<int>(system::errc::invalid_argument), "cppmsboost::invalid_thread_argument")
        {}

        invalid_thread_argument( int ev )
        : base_type(ev, "cppmsboost::invalid_thread_argument")
        {
        }
        invalid_thread_argument( int ev, const char * what_arg )
        : base_type(ev, what_arg)
        {
        }
        invalid_thread_argument( int ev, const std::string & what_arg )
        : base_type(ev, what_arg)
        {
        }

    };

    class BOOST_SYMBOL_VISIBLE thread_permission_error:
        public thread_exception
    {
          typedef thread_exception base_type;
    public:
          thread_permission_error()
          : base_type(static_cast<int>(system::errc::permission_denied), "cppmsboost::thread_permission_error")
          {}

          thread_permission_error( int ev )
          : base_type(ev, "cppmsboost::thread_permission_error")
          {
          }
          thread_permission_error( int ev, const char * what_arg )
          : base_type(ev, what_arg)
          {
          }
          thread_permission_error( int ev, const std::string & what_arg )
          : base_type(ev, what_arg)
          {
          }

    };

} // namespace cppmsboost

#include <boost/config/abi_suffix.hpp>

#endif
