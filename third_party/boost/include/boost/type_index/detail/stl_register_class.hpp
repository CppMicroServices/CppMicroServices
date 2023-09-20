//
// Copyright 2013-2020 Antony Polukhin.
//
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_TYPE_INDEX_STL_REGISTER_CLASS_HPP
#define BOOST_TYPE_INDEX_STL_REGISTER_CLASS_HPP

/// \file stl_register_class.hpp
/// \brief Contains BOOST_TYPE_INDEX_REGISTER_CLASS macro implementation that uses cppmsboost::typeindex::stl_type_index.
/// Not intended for inclusion from user's code.

#include <boost/type_index/stl_type_index.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif

namespace cppmsboost { namespace typeindex { namespace detail {

template <class T>
inline const stl_type_index::type_info_t& stl_construct_typeid_ref(const T*) BOOST_NOEXCEPT {
    return typeid(T);
}

}}} // namespace cppmsboost::typeindex::detail

/// @cond
#define BOOST_TYPE_INDEX_REGISTER_CLASS                                                                                     \
    virtual const cppmsboost::typeindex::stl_type_index::type_info_t& boost_type_index_type_id_runtime_() const BOOST_NOEXCEPT { \
        return cppmsboost::typeindex::detail::stl_construct_typeid_ref(this);                                                    \
    }                                                                                                                       \
/**/
/// @endcond

#endif // BOOST_TYPE_INDEX_STL_REGISTER_CLASS_HPP

