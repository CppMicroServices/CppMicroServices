/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "BundleLoader.hpp"
#include <regex>
#if defined(_WIN32)
#  include <Windows.h>
#else
#  include <dlfcn.h>
#endif

namespace cppmicroservices {
namespace scrimpl {

#if defined(_WIN32)
/**
 * Utility function to convert UTF8 string to UTF16 string. This method is copied from
 * the CppMicroServices framework codebase(util/src/String.cpp).
 */
std::wstring UTF8StrToWStr(const std::string& inStr)
{
  if (inStr.empty()) {
    return std::wstring();
  }
  int wchar_count = MultiByteToWideChar(CP_UTF8, 0, inStr.c_str(), -1, NULL, 0);
  std::unique_ptr<wchar_t[]> wBuf(new wchar_t[wchar_count]);
  wchar_count =
    MultiByteToWideChar(CP_UTF8, 0, inStr.c_str(), -1, wBuf.get(), wchar_count);
  if (wchar_count == 0) {
    std::invalid_argument("Failed to convert " + inStr + " to UTF16.");
  }
  return wBuf.get();
}
#endif

std::tuple<std::function<ComponentInstance*(void)>,
           std::function<void(ComponentInstance*)>>
GetComponentCreatorDeletors(const std::string& compName,
                            const cppmicroservices::Bundle& fromBundle)
{
  // cannot use bundle id as key because id is reused when the framework is restarted.
  // strings are not optimal but will work fine as long as a binary is not unloaded
  // from the process.
  // Note: This code is a temporary hack until the core framework supports Bundle#load API.
  static Guarded<std::map<std::string, void*>>
    bundleBinaries; ///< map of bundle location and handle pairs
  const auto bundleLoc = fromBundle.GetLocation();

  void* handle = nullptr;
  if (bundleBinaries.lock()->count(bundleLoc) != 0u) {
    handle = bundleBinaries.lock()->at(bundleLoc);
  } else {
#if defined(_WIN32)
    std::wstring bundlePathWstr = UTF8StrToWStr(fromBundle.GetLocation());
    handle = reinterpret_cast<void*>(LoadLibraryW(bundlePathWstr.c_str()));
    if (handle == nullptr) {
      std::string errMsg("Unable to load bundle binary ");
      errMsg += fromBundle.GetLocation();
      errMsg += ". Error: ";
      errMsg += std::to_string(GetLastError());
      throw std::runtime_error(errMsg);
    }
#else
    handle = dlopen(fromBundle.GetLocation().c_str(), RTLD_LAZY | RTLD_LOCAL);
    if (handle == nullptr) {
      std::string errMsg("Unable to load bundle binary ");
      errMsg += fromBundle.GetLocation();
      errMsg += ". Error: ";
      const char* dlErrMsg = dlerror();
      errMsg += (dlErrMsg) ? dlErrMsg : "none";
      throw std::runtime_error(errMsg);
    }
#endif
    bundleBinaries.lock()->emplace(bundleLoc, handle);
  }

  const std::string symbolName =
    std::regex_replace(compName, std::regex("::"), "_");
  const std::string newInstanceFuncName("NewInstance_" + symbolName);
  const std::string deleteInstanceFuncName("DeleteInstance_" + symbolName);
#if defined(_WIN32)
  void* sym = reinterpret_cast<void*>(GetProcAddress(
    reinterpret_cast<HMODULE>(handle), newInstanceFuncName.c_str()));
  void* delsym = reinterpret_cast<void*>(GetProcAddress(
    reinterpret_cast<HMODULE>(handle), deleteInstanceFuncName.c_str()));
  if (sym == nullptr || delsym == nullptr) {
    std::string errMsg("Unable to find entry-point functions in bundle ");
    errMsg += fromBundle.GetLocation();
    errMsg += ". Error code: ";
    errMsg += std::to_string(GetLastError());
    throw std::runtime_error(errMsg);
  }
#else
  void* sym = dlsym(handle, newInstanceFuncName.c_str());
  void* delsym = dlsym(handle, deleteInstanceFuncName.c_str());
  if (sym == nullptr || delsym == nullptr) {
    std::string errMsg("Unable to find entry-point functions in bundle ");
    errMsg += fromBundle.GetLocation();
    errMsg += ". Error: ";
    const char* dlErrMsg = dlerror();
    errMsg += (dlErrMsg) ? dlErrMsg : "none";
    throw std::runtime_error(errMsg);
  }
#endif

  return std::make_tuple(
    reinterpret_cast<ComponentInstance* (*)(void)>(sym),     // NOLINT
    reinterpret_cast<void (*)(ComponentInstance*)>(delsym)); // NOLINT
}
}
}
