/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/


#ifndef CPPMICROSERVICES_UTILS_H
#define CPPMICROSERVICES_UTILS_H

#include "cppmicroservices/FrameworkConfig.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

#if defined(__ANDROID__)
  #include <sstream>
#endif

//-------------------------------------------------------------------
// File system functions
//-------------------------------------------------------------------

namespace cppmicroservices {

const char DIR_SEP_WIN32 = '\\';
const char DIR_SEP_POSIX = '/';

#ifdef US_PLATFORM_WINDOWS
const char DIR_SEP = DIR_SEP_WIN32;
#else
const char DIR_SEP = DIR_SEP_POSIX;
#endif

namespace fs {

// Platform agnostic way to get the current working directory.
// Supports Linux, Mac, and Windows.
std::string GetCurrentWorkingDirectory();
bool not_found_error(int errval);
bool Exists(const std::string& path);

bool IsDirectory(const std::string& path);
bool IsFile(const std::string& path);
bool IsRelative(const std::string& path);

std::string GetAbsolute(const std::string& path);

void MakePath(const std::string& path);

US_Framework_EXPORT void RemoveDirectoryRecursive(const std::string& path);

} // namespace fs


//-------------------------------------------------------------------
// File type checking
//-------------------------------------------------------------------

bool IsSharedLibrary(const std::string& location);

bool IsBundleFile(const std::string& location);

//-------------------------------------------------------------------
// Framework storage
//-------------------------------------------------------------------

class CoreBundleContext;

extern const std::string FWDIR_DEFAULT;

std::string GetFrameworkDir(CoreBundleContext* ctx);

/**
 * Check for local file storage directory.
 *
 * @return A directory path or an empty string if no storage is available.
 */
std::string GetFileStorage(CoreBundleContext* ctx, const std::string& name, bool create = true);

//-------------------------------------------------------------------
// Generic utility functions
//-------------------------------------------------------------------

// A convenient way to construct a shared_ptr holding an array
template<typename T> std::shared_ptr<T> make_shared_array(std::size_t size)
{
  return std::shared_ptr<T>(new T[size], std::default_delete<T[]>());
}

// Platform agnostic way to get the current working directory.
// Supports Linux, Mac, and Windows.
std::string GetCurrentWorkingDirectory();

void TerminateForDebug(const std::exception_ptr ex);

//-------------------------------------------------------------------
// Error handling
//-------------------------------------------------------------------

int GetLastErrorNo();
std::string GetLastErrorStr();

//-------------------------------------------------------------------
// Android Compatibility functions
//-------------------------------------------------------------------

/**
 * Compatibility functions to replace "std::to_string(...)" functions
 * on Android, since the latest Android NDKs lack "std::to_string(...)"
 * support.
 */

template <typename T>
std::string ToString(T val)
{
#if defined(__ANDROID__)
  std::ostringstream os;
  os << val;
  return os.str();
#else
  return std::to_string(val);
#endif
}


} // namespace cppmicroservices

#endif // CPPMICROSERVICES_UTILS_H
