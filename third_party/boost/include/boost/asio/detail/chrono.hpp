//
// detail/chrono.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2020 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_CHRONO_HPP
#define BOOST_ASIO_DETAIL_CHRONO_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/config.hpp>

#if defined(BOOST_ASIO_HAS_STD_CHRONO)
# include <chrono>
#elif defined(BOOST_ASIO_HAS_BOOST_CHRONO)
# include <boost/chrono/system_clocks.hpp>
#endif // defined(BOOST_ASIO_HAS_BOOST_CHRONO)

namespace cppmsboost {
namespace asio {
namespace chrono {

#if defined(BOOST_ASIO_HAS_STD_CHRONO)
using std::chrono::duration;
using std::chrono::time_point;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::minutes;
using std::chrono::hours;
using std::chrono::time_point_cast;
#if defined(BOOST_ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
typedef std::chrono::monotonic_clock steady_clock;
#else // defined(BOOST_ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
using std::chrono::steady_clock;
#endif // defined(BOOST_ASIO_HAS_STD_CHRONO_MONOTONIC_CLOCK)
using std::chrono::system_clock;
using std::chrono::high_resolution_clock;
#elif defined(BOOST_ASIO_HAS_BOOST_CHRONO)
using cppmsboost::chrono::duration;
using cppmsboost::chrono::time_point;
using cppmsboost::chrono::duration_cast;
using cppmsboost::chrono::nanoseconds;
using cppmsboost::chrono::microseconds;
using cppmsboost::chrono::milliseconds;
using cppmsboost::chrono::seconds;
using cppmsboost::chrono::minutes;
using cppmsboost::chrono::hours;
using cppmsboost::chrono::time_point_cast;
using cppmsboost::chrono::system_clock;
using cppmsboost::chrono::steady_clock;
using cppmsboost::chrono::high_resolution_clock;
#endif // defined(BOOST_ASIO_HAS_BOOST_CHRONO)

} // namespace chrono
} // namespace asio
} // namespace cppmsboost

#endif // BOOST_ASIO_DETAIL_CHRONO_HPP
