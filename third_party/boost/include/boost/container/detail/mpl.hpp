//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_MPL_HPP
#define BOOST_CONTAINER_CONTAINER_DETAIL_MPL_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/move/detail/type_traits.hpp>
#include <boost/intrusive/detail/mpl.hpp>

#include <cstddef>

namespace cppmsboost {
namespace container {
namespace dtl {

using cppmsboost::move_detail::integral_constant;
using cppmsboost::move_detail::true_type;
using cppmsboost::move_detail::false_type;
using cppmsboost::move_detail::enable_if_c;
using cppmsboost::move_detail::enable_if;
using cppmsboost::move_detail::enable_if_convertible;
using cppmsboost::move_detail::disable_if_c;
using cppmsboost::move_detail::disable_if;
using cppmsboost::move_detail::disable_if_convertible;
using cppmsboost::move_detail::is_convertible;
using cppmsboost::move_detail::if_c;
using cppmsboost::move_detail::if_;
using cppmsboost::move_detail::identity;
using cppmsboost::move_detail::bool_;
using cppmsboost::move_detail::true_;
using cppmsboost::move_detail::false_;
using cppmsboost::move_detail::yes_type;
using cppmsboost::move_detail::no_type;
using cppmsboost::move_detail::bool_;
using cppmsboost::move_detail::true_;
using cppmsboost::move_detail::false_;
using cppmsboost::move_detail::unvoid_ref;
using cppmsboost::move_detail::and_;
using cppmsboost::move_detail::or_;
using cppmsboost::move_detail::not_;
using cppmsboost::move_detail::enable_if_and;
using cppmsboost::move_detail::disable_if_and;
using cppmsboost::move_detail::enable_if_or;
using cppmsboost::move_detail::disable_if_or;
using cppmsboost::move_detail::remove_const;

template <class FirstType>
struct select1st
{
   typedef FirstType type;

   template<class T>
   BOOST_CONTAINER_FORCEINLINE const type& operator()(const T& x) const
   {  return x.first;   }

   template<class T>
   BOOST_CONTAINER_FORCEINLINE type& operator()(T& x)
   {  return const_cast<type&>(x.first);   }
};


template<typename T>
struct void_t { typedef void type; };

template <class T, class=void>
struct is_transparent_base
{
   static const bool value = false;
};

template <class T>
struct is_transparent_base<T, typename void_t<typename T::is_transparent>::type>
{
   static const bool value = true;
};

template <class T>
struct is_transparent
   : is_transparent_base<T>
{};

template <typename C, class /*Dummy*/, typename R>
struct enable_if_transparent
   : cppmsboost::move_detail::enable_if_c<dtl::is_transparent<C>::value, R>
{};

#ifndef BOOST_CONTAINER_NO_CXX17_CTAD

// void_t (void_t for C++11)
template<typename...> using variadic_void_t = void;

// Trait to detect Allocator-like types.
template<typename Allocator, typename = void>
struct is_allocator
{
   static const bool value = false;
};

template <typename T>
T&& ctad_declval();

template<typename Allocator>
struct is_allocator < Allocator,
   variadic_void_t< typename Allocator::value_type
                  , decltype(ctad_declval<Allocator&>().allocate(size_t{})) >>
{
   static const bool value = true;
};

template<class T>
using require_allocator_t = typename enable_if_c<is_allocator<T>::value, T>::type;

template<class T>
using require_nonallocator_t = typename enable_if_c<!is_allocator<T>::value, T>::type;

#endif

}  //namespace dtl {
}  //namespace container {
}  //namespace cppmsboost {

#include <boost/container/detail/config_end.hpp>

#endif   //#ifndef BOOST_CONTAINER_CONTAINER_DETAIL_MPL_HPP

