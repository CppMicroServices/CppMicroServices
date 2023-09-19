/*
 * Copyright 2010 Vicente J. Botet Escriba
 * Copyright 2015, 2017 Andrey Semashev
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef BOOST_WINAPI_HEAP_MEMORY_HPP_INCLUDED_
#define BOOST_WINAPI_HEAP_MEMORY_HPP_INCLUDED_

#include <boost/winapi/basic_types.hpp>

#ifdef BOOST_HAS_PRAGMA_ONCE
#pragma once
#endif

#if !defined( BOOST_USE_WINDOWS_H )
#undef HeapAlloc
extern "C" {

#if BOOST_WINAPI_PARTITION_DESKTOP_SYSTEM
BOOST_SYMBOL_IMPORT cppmsboost::winapi::DWORD_ BOOST_WINAPI_WINAPI_CC
GetProcessHeaps(cppmsboost::winapi::DWORD_ NumberOfHeaps, cppmsboost::winapi::PHANDLE_ ProcessHeaps);
#endif // BOOST_WINAPI_PARTITION_DESKTOP_SYSTEM

BOOST_SYMBOL_IMPORT cppmsboost::winapi::HANDLE_ BOOST_WINAPI_WINAPI_CC
GetProcessHeap(BOOST_WINAPI_DETAIL_VOID);

BOOST_SYMBOL_IMPORT cppmsboost::winapi::LPVOID_ BOOST_WINAPI_WINAPI_CC
HeapAlloc(
    cppmsboost::winapi::HANDLE_ hHeap,
    cppmsboost::winapi::DWORD_ dwFlags,
    cppmsboost::winapi::SIZE_T_ dwBytes);

BOOST_SYMBOL_IMPORT cppmsboost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC
HeapFree(
    cppmsboost::winapi::HANDLE_ hHeap,
    cppmsboost::winapi::DWORD_ dwFlags,
    cppmsboost::winapi::LPVOID_ lpMem);

BOOST_SYMBOL_IMPORT cppmsboost::winapi::LPVOID_ BOOST_WINAPI_WINAPI_CC
HeapReAlloc(
    cppmsboost::winapi::HANDLE_ hHeap,
    cppmsboost::winapi::DWORD_ dwFlags,
    cppmsboost::winapi::LPVOID_ lpMem,
    cppmsboost::winapi::SIZE_T_ dwBytes);

#if BOOST_WINAPI_PARTITION_APP_SYSTEM
BOOST_SYMBOL_IMPORT cppmsboost::winapi::HANDLE_ BOOST_WINAPI_WINAPI_CC
HeapCreate(
    cppmsboost::winapi::DWORD_ flOptions,
    cppmsboost::winapi::SIZE_T_ dwInitialSize,
    cppmsboost::winapi::SIZE_T_ dwMaximumSize);

BOOST_SYMBOL_IMPORT cppmsboost::winapi::BOOL_ BOOST_WINAPI_WINAPI_CC
HeapDestroy(cppmsboost::winapi::HANDLE_ hHeap);
#endif // BOOST_WINAPI_PARTITION_APP_SYSTEM

} // extern "C"
#endif // !defined( BOOST_USE_WINDOWS_H )

namespace cppmsboost {
namespace winapi {

#if BOOST_WINAPI_PARTITION_DESKTOP_SYSTEM
using ::GetProcessHeaps;
#endif

using ::GetProcessHeap;
using ::HeapAlloc;
using ::HeapFree;
using ::HeapReAlloc;

#if BOOST_WINAPI_PARTITION_APP_SYSTEM
using ::HeapCreate;
using ::HeapDestroy;
#endif

}
}

#endif // BOOST_WINAPI_HEAP_MEMORY_HPP_INCLUDED_
