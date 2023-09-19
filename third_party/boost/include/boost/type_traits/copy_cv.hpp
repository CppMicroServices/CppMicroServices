#ifndef BOOST_TYPE_TRAITS_COPY_CV_HPP_INCLUDED
#define BOOST_TYPE_TRAITS_COPY_CV_HPP_INCLUDED

//
//  Copyright 2015 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//

#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_volatile.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/add_volatile.hpp>
#include <boost/type_traits/conditional.hpp>

namespace cppmsboost
{

template<class T, class U> struct copy_cv
{
private:

    typedef typename cppmsboost::conditional<cppmsboost::is_const<U>::value, typename cppmsboost::add_const<T>::type, T>::type CT;

public:

    typedef typename cppmsboost::conditional<cppmsboost::is_volatile<U>::value, typename cppmsboost::add_volatile<CT>::type, CT>::type type;
};

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

   template <class T, class U> using copy_cv_t = typename copy_cv<T, U>::type;

#endif

} // namespace cppmsboost

#endif // #ifndef BOOST_TYPE_TRAITS_COPY_CV_HPP_INCLUDED
