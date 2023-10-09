
//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#ifndef BOOST_TT_IS_STATELESS_HPP_INCLUDED
#define BOOST_TT_IS_STATELESS_HPP_INCLUDED

#include <boost/type_traits/has_trivial_constructor.hpp>
#include <boost/type_traits/has_trivial_copy.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/is_empty.hpp>
#include <boost/config.hpp>

namespace cppmsboost {

template <typename T>
struct is_stateless
 : public integral_constant<bool,  
      (::cppmsboost::has_trivial_constructor<T>::value
      && ::cppmsboost::has_trivial_copy<T>::value
      && ::cppmsboost::has_trivial_destructor<T>::value
      && ::cppmsboost::is_class<T>::value
      && ::cppmsboost::is_empty<T>::value)>
{};

} // namespace cppmsboost

#endif // BOOST_TT_IS_STATELESS_HPP_INCLUDED
