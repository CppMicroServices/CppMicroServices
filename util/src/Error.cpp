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

#include "cppmicroservices/util/Error.h"

#include <cppmicroservices/GlobalConfig.h>

#ifdef US_PLATFORM_POSIX
  #include <errno.h>
  #include <string.h>

#else
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <crtdbg.h>
#endif

namespace cppmicroservices {

namespace util {

std::string GetLastWin32ErrorStr()
{
#ifdef US_PLATFORM_WINDOWS
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  DWORD rc = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&lpMsgBuf),
        0,
        NULL
        );

  // If FormatMessage fails using FORMAT_MESSAGE_ALLOCATE_BUFFER
  // it means that the size of the error message exceeds an internal
  // buffer limit (128 kb according to MSDN) and lpMsgBuf will be
  // uninitialized.
  // Inform the caller that the error message couldn't be retrieved.
  if (rc == 0)
  {
    return std::string("Failed to retrieve error message.");
  }

  std::string errMsg(reinterpret_cast<LPCTSTR>(lpMsgBuf));

  LocalFree(lpMsgBuf);
  return errMsg;
#else
  return std::string();
#endif
}

std::string GetLastCErrorStr()
{
  char errorString[128];
#if ((_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE) || defined(US_PLATFORM_APPLE)
  // This is the XSI strerror_r version
  if (strerror_r(errno, errorString, sizeof errorString))
  {
    return "Unknown error";
  }
  return errorString;
#elif defined(US_PLATFORM_WINDOWS)
  if (strerror_s(errorString, sizeof errorString, errno))
  {
    return "Unknown error";
  }
  return errorString;
#else
  return strerror_r(errno, errorString, sizeof errorString);
#endif
}

US_MSVC_PUSH_DISABLE_WARNING(4715) // 'function' : not all control paths return a value
std::string GetExceptionStr(const std::exception_ptr& exc)
{
  if (!exc)
  {
    return std::string();
  }

  try
  {
    std::rethrow_exception(exc);
  }
  catch (const std::exception& e)
  {
    return e.what();
  }
  catch (...)
  {
    return "unknown";
  }
}
US_MSVC_POP_WARNING

std::string GetLastExceptionStr()
{
  return GetExceptionStr(std::current_exception());
}

} // namespace util
} // namespace cppmicroservices
