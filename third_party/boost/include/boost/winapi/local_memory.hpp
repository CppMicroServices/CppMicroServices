/*
 * Copyright 2010 Vicente J. Botet Escriba
 * Copyright 2015 Andrey Semashev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_LOCAL_MEMORY_HPP_INCLUDED_
#define BOOST_WINAPI_LOCAL_MEMORY_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if BOOST_WINAPI_PARTITION_APP_SYSTEM

#if !defined( BOOST_USE_WINDOWS_H )
namespace cppmsboost { namespace winapi {
typedef HANDLE_ HLOCAL_;
}}

extern "C" {
BOOST_SYMBOL_IMPORT cppmsboost::winapi::HLOCAL_ BOOST_WINAPI_WINAPI_CC
LocalAlloc(
    cppmsboost::winapi::UINT_ uFlags,
    cppmsboost::winapi::SIZE_T_ uBytes);

BOOST_SYMBOL_IMPORT cppmsboost::winapi::HLOCAL_ BOOST_WINAPI_WINAPI_CC
LocalReAlloc(
    cppmsboost::winapi::HLOCAL_ hMem,
    cppmsboost::winapi::SIZE_T_ uBytes,
    cppmsboost::winapi::UINT_ uFlags);

BOOST_SYMBOL_IMPORT cppmsboost::winapi::HLOCAL_ BOOST_WINAPI_WINAPI_CC LocalFree(cppmsboost::winapi::HLOCAL_ hMem);
} // extern "C"
#endif

namespace cppmsboost {
namespace winapi {
#if defined( BOOST_USE_WINDOWS_H )
typedef ::HLOCAL HLOCAL_;
#endif
using ::LocalAlloc;
using ::LocalReAlloc;
using ::LocalFree;
}
}

#endif // BOOST_WINAPI_PARTITION_APP_SYSTEM
#endif // BOOST_WINAPI_LOCAL_MEMORY_HPP_INCLUDED_
