/*
Copyright 2012-2019 Glen Joseph Fernandes
(glenjofe@gmail.com)

Distributed under the Boost Software License, Version 1.0.
(http://www.boost.org/LICENSE_1_0.txt)
*/
#ifndef BOOST_SMART_PTR_MAKE_SHARED_ARRAY_HPP
#define BOOST_SMART_PTR_MAKE_SHARED_ARRAY_HPP

#include <boost/core/default_allocator.hpp>
#include <boost/smart_ptr/allocate_shared_array.hpp>

namespace cppmsboost {

template<class T>
inline typename enable_if_<is_bounded_array<T>::value, shared_ptr<T> >::type
make_shared()
{
    return cppmsboost::allocate_shared<T>(cppmsboost::default_allocator<typename
        detail::sp_array_element<T>::type>());
}

template<class T>
inline typename enable_if_<is_bounded_array<T>::value, shared_ptr<T> >::type
make_shared(const typename remove_extent<T>::type& value)
{
    return cppmsboost::allocate_shared<T>(cppmsboost::default_allocator<typename
        detail::sp_array_element<T>::type>(), value);
}

template<class T>
inline typename enable_if_<is_unbounded_array<T>::value, shared_ptr<T> >::type
make_shared(std::size_t size)
{
    return cppmsboost::allocate_shared<T>(cppmsboost::default_allocator<typename
        detail::sp_array_element<T>::type>(), size);
}

template<class T>
inline typename enable_if_<is_unbounded_array<T>::value, shared_ptr<T> >::type
make_shared(std::size_t size, const typename remove_extent<T>::type& value)
{
    return cppmsboost::allocate_shared<T>(cppmsboost::default_allocator<typename
        detail::sp_array_element<T>::type>(), size, value);
}

template<class T>
inline typename enable_if_<is_bounded_array<T>::value, shared_ptr<T> >::type
make_shared_noinit()
{
    return cppmsboost::allocate_shared_noinit<T>(cppmsboost::default_allocator<typename
        detail::sp_array_element<T>::type>());
}

template<class T>
inline typename enable_if_<is_unbounded_array<T>::value, shared_ptr<T> >::type
make_shared_noinit(std::size_t size)
{
    return cppmsboost::allocate_shared_noinit<T>(cppmsboost::default_allocator<typename
        detail::sp_array_element<T>::type>(), size);
}

} /* boost */

#endif
