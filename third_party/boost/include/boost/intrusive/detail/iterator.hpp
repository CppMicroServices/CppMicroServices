/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga  2014-2014
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_DETAIL_ITERATOR_HPP
#define BOOST_INTRUSIVE_DETAIL_ITERATOR_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <cstddef>
#include <boost/intrusive/detail/std_fwd.hpp>
#include <boost/intrusive/detail/workaround.hpp>
#include <boost/move/detail/iterator_traits.hpp>
#include <boost/move/detail/meta_utils_core.hpp>

namespace cppmsboost{
namespace iterators{

struct incrementable_traversal_tag;
struct single_pass_traversal_tag;
struct forward_traversal_tag;
struct bidirectional_traversal_tag;
struct random_access_traversal_tag;

namespace detail{

template <class Category, class Traversal>
struct iterator_category_with_traversal;

} //namespace cppmsboost{
} //namespace iterators{
} //namespace detail{

namespace cppmsboost {
namespace intrusive {

using cppmsboost::movelib::iterator_traits;

////////////////////
//    iterator
////////////////////
template<class Category, class T, class Difference, class Pointer, class Reference>
struct iterator
{
   typedef Category     iterator_category;
   typedef T            value_type;
   typedef Difference   difference_type;
   typedef Pointer      pointer;
   typedef Reference    reference;
};

////////////////////////////////////////
//    iterator_[dis|en]able_if_boost_iterator
////////////////////////////////////////
template<class I>
struct is_boost_iterator
{
   static const bool value = false;
};

template<class Category, class Traversal>
struct is_boost_iterator< cppmsboost::iterators::detail::iterator_category_with_traversal<Category, Traversal> >
{
   static const bool value = true;
};

template<class I, class R = void>
struct iterator_enable_if_boost_iterator
   : ::cppmsboost::move_detail::enable_if_c
      < is_boost_iterator<typename cppmsboost::intrusive::iterator_traits<I>::iterator_category >::value
      , R>
{};

////////////////////////////////////////
//    iterator_[dis|en]able_if_tag
////////////////////////////////////////
template<class I, class Tag, class R = void>
struct iterator_enable_if_tag
   : ::cppmsboost::move_detail::enable_if_c
      < ::cppmsboost::move_detail::is_same
         < typename cppmsboost::intrusive::iterator_traits<I>::iterator_category 
         , Tag
         >::value
         , R>
{};

template<class I, class Tag, class R = void>
struct iterator_disable_if_tag
   : ::cppmsboost::move_detail::enable_if_c
      < !::cppmsboost::move_detail::is_same
         < typename cppmsboost::intrusive::iterator_traits<I>::iterator_category 
         , Tag
         >::value
         , R>
{};

////////////////////////////////////////
//    iterator_[dis|en]able_if_tag
////////////////////////////////////////
template<class I, class Tag, class Tag2, class R = void>
struct iterator_enable_if_convertible_tag
   : ::cppmsboost::move_detail::enable_if_c
      < ::cppmsboost::move_detail::is_same_or_convertible
         < typename cppmsboost::intrusive::iterator_traits<I>::iterator_category 
         , Tag
         >::value &&
        !::cppmsboost::move_detail::is_same_or_convertible
         < typename cppmsboost::intrusive::iterator_traits<I>::iterator_category 
         , Tag2
         >::value
         , R>
{};

////////////////////////////////////////
//    iterator_[dis|en]able_if_tag_difference_type
////////////////////////////////////////
template<class I, class Tag>
struct iterator_enable_if_tag_difference_type
   : iterator_enable_if_tag<I, Tag, typename cppmsboost::intrusive::iterator_traits<I>::difference_type>
{};

template<class I, class Tag>
struct iterator_disable_if_tag_difference_type
   : iterator_disable_if_tag<I, Tag, typename cppmsboost::intrusive::iterator_traits<I>::difference_type>
{};

////////////////////
//    advance
////////////////////

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_tag<InputIt, std::input_iterator_tag>::type
   iterator_advance(InputIt& it, Distance n)
{
   while(n--)
      ++it;
}

template<class InputIt, class Distance>
typename iterator_enable_if_tag<InputIt, std::forward_iterator_tag>::type
   iterator_advance(InputIt& it, Distance n)
{
   while(n--)
      ++it;
}

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_tag<InputIt, std::bidirectional_iterator_tag>::type
   iterator_advance(InputIt& it, Distance n)
{
   for (; 0 < n; --n)
      ++it;
   for (; n < 0; ++n)
      --it;
}

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_tag<InputIt, std::random_access_iterator_tag>::type
   iterator_advance(InputIt& it, Distance n)
{
   it += n;
}

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_convertible_tag
   <InputIt, const cppmsboost::iterators::incrementable_traversal_tag&, const cppmsboost::iterators::single_pass_traversal_tag&>::type
   iterator_advance(InputIt& it, Distance n)
{
   while(n--)
      ++it;
}

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_convertible_tag
   <InputIt, const cppmsboost::iterators::single_pass_traversal_tag &, const cppmsboost::iterators::forward_traversal_tag&>::type
   iterator_advance(InputIt& it, Distance n)
{
   while(n--)
      ++it;
}

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_convertible_tag
   <InputIt, const cppmsboost::iterators::forward_traversal_tag&, const cppmsboost::iterators::bidirectional_traversal_tag&>::type
   iterator_advance(InputIt& it, Distance n)
{
   while(n--)
      ++it;
}

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_convertible_tag
   <InputIt, const cppmsboost::iterators::bidirectional_traversal_tag&, const cppmsboost::iterators::random_access_traversal_tag&>::type
   iterator_advance(InputIt& it, Distance n)
{
   for (; 0 < n; --n)
      ++it;
   for (; n < 0; ++n)
      --it;
}

class fake{};

template<class InputIt, class Distance>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_convertible_tag
   <InputIt, const cppmsboost::iterators::random_access_traversal_tag&, const fake&>::type
   iterator_advance(InputIt& it, Distance n)
{
   it += n;
}

////////////////////
//    distance
////////////////////
template<class InputIt> inline
typename iterator_disable_if_tag_difference_type
   <InputIt, std::random_access_iterator_tag>::type
      iterator_distance(InputIt first, InputIt last)
{
   typename iterator_traits<InputIt>::difference_type off = 0;
   while(first != last){
      ++off;
      ++first;
   }
   return off;
}

template<class InputIt>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_enable_if_tag_difference_type
   <InputIt, std::random_access_iterator_tag>::type
      iterator_distance(InputIt first, InputIt last)
{
   typename iterator_traits<InputIt>::difference_type off = last - first;
   return off;
}

template<class I>
BOOST_INTRUSIVE_FORCEINLINE typename iterator_traits<I>::pointer iterator_arrow_result(const I &i)
{  return i.operator->();  }

template<class T>
BOOST_INTRUSIVE_FORCEINLINE T * iterator_arrow_result(T *p)
{  return p;   }

} //namespace intrusive
} //namespace cppmsboost

#endif //BOOST_INTRUSIVE_DETAIL_ITERATOR_HPP
