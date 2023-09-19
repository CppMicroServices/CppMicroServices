//
// placeholders.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_PLACEHOLDERS_HPP
#define BOOST_ASIO_PLACEHOLDERS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_BOOST_BIND)
# include <boost/bind/arg.hpp>
#endif // defined(BOOST_ASIO_HAS_BOOST_BIND)

#include <boost/asio/detail/push_options.hpp>

namespace cppmsboost {
namespace asio {
namespace placeholders {

#if defined(GENERATING_DOCUMENTATION)

/// An argument placeholder, for use with cppmsboost::bind(), that corresponds to
/// the error argument of a handler for any of the asynchronous functions.
unspecified error;

/// An argument placeholder, for use with cppmsboost::bind(), that corresponds to
/// the bytes_transferred argument of a handler for asynchronous functions such
/// as cppmsboost::asio::basic_stream_socket::async_write_some or
/// cppmsboost::asio::async_write.
unspecified bytes_transferred;

/// An argument placeholder, for use with cppmsboost::bind(), that corresponds to
/// the iterator argument of a handler for asynchronous functions such as
/// cppmsboost::asio::async_connect.
unspecified iterator;

/// An argument placeholder, for use with cppmsboost::bind(), that corresponds to
/// the results argument of a handler for asynchronous functions such as
/// cppmsboost::asio::basic_resolver::async_resolve.
unspecified results;

/// An argument placeholder, for use with cppmsboost::bind(), that corresponds to
/// the results argument of a handler for asynchronous functions such as
/// cppmsboost::asio::async_connect.
unspecified endpoint;

/// An argument placeholder, for use with cppmsboost::bind(), that corresponds to
/// the signal_number argument of a handler for asynchronous functions such as
/// cppmsboost::asio::signal_set::async_wait.
unspecified signal_number;

#elif defined(BOOST_ASIO_HAS_BOOST_BIND)
# if defined(__BORLANDC__) || defined(__GNUC__)

inline cppmsboost::arg<1> error()
{
  return cppmsboost::arg<1>();
}

inline cppmsboost::arg<2> bytes_transferred()
{
  return cppmsboost::arg<2>();
}

inline cppmsboost::arg<2> iterator()
{
  return cppmsboost::arg<2>();
}

inline cppmsboost::arg<2> results()
{
  return cppmsboost::arg<2>();
}

inline cppmsboost::arg<2> endpoint()
{
  return cppmsboost::arg<2>();
}

inline cppmsboost::arg<2> signal_number()
{
  return cppmsboost::arg<2>();
}

# else

namespace detail
{
  template <int Number>
  struct placeholder
  {
    static cppmsboost::arg<Number>& get()
    {
      static cppmsboost::arg<Number> result;
      return result;
    }
  };
}

#  if defined(BOOST_ASIO_MSVC) && (BOOST_ASIO_MSVC < 1400)

static cppmsboost::arg<1>& error
  = cppmsboost::asio::placeholders::detail::placeholder<1>::get();
static cppmsboost::arg<2>& bytes_transferred
  = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
static cppmsboost::arg<2>& iterator
  = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
static cppmsboost::arg<2>& results
  = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
static cppmsboost::arg<2>& endpoint
  = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
static cppmsboost::arg<2>& signal_number
  = cppmsboost::asio::placeholders::detail::placeholder<2>::get();

#  else

namespace
{
  cppmsboost::arg<1>& error
    = cppmsboost::asio::placeholders::detail::placeholder<1>::get();
  cppmsboost::arg<2>& bytes_transferred
    = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
  cppmsboost::arg<2>& iterator
    = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
  cppmsboost::arg<2>& results
    = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
  cppmsboost::arg<2>& endpoint
    = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
  cppmsboost::arg<2>& signal_number
    = cppmsboost::asio::placeholders::detail::placeholder<2>::get();
} // namespace

#  endif
# endif
#endif

} // namespace placeholders
} // namespace asio
} // namespace cppmsboost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_PLACEHOLDERS_HPP
