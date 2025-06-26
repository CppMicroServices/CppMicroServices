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
#include "cppmicroservices/util/String.h"

#ifdef US_PLATFORM_POSIX
#    include <cerrno>
#    include <cstring>

#else
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <crtdbg.h>
#    include <windows.h>
#endif
#include <array>
#include <string>

namespace {
    constexpr std::size_t kErrorBufferSize = 128;
}

namespace cppmicroservices::util
{

#ifdef US_PLATFORM_WINDOWS
    std::string
    GetLastWin32ErrorStr()
    {
        DWORD errorCode = ::GetLastError();
        LPWSTR wideMsgBuf = nullptr;

        DWORD rc = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                                        | FORMAT_MESSAGE_IGNORE_INSERTS,
                                    nullptr,
                                    errorCode,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                    reinterpret_cast<LPWSTR>(&wideMsgBuf),
                                    0,
                                    nullptr);

        // If FormatMessage fails using FORMAT_MESSAGE_ALLOCATE_BUFFER
        // it means that the size of the error message exceeds an internal
        // buffer limit (128 kb according to MSDN) and lpMsgBuf will be
        // uninitialized.
        // Inform the caller that the error message couldn't be retrieved.
        if (rc == 0)
        {
            return std::string{"Failed to retrieve error message."};
        }

        std::wstring wideMsg(wideMsgBuf);
        ::LocalFree(wideMsgBuf);

        return ToUTF8String(wideMsg);
    }
#endif

    std::string
    GetLastCErrorStr()
    {
        std::array<char, kErrorBufferSize> errorString{};
#if ((_POSIX_C_SOURCE >= 200112L) && !_GNU_SOURCE) || defined(US_PLATFORM_APPLE)
        // This is the XSI strerror_r version
        int en = errno;
        int r = strerror_r(errno, errorString.data(), errorString.size());
        if (r)
        {
            std::string errMsg = "Unknown error " + util::ToString(en) + ": strerror_r failed with error code ";
            if (r < 0)
            {
                errMsg += util::ToString(static_cast<int>(errno));
            }
            else
            {
                errMsg += util::ToString(r);
            }
            return errMsg;
        }
        return errorString;
#elif defined(US_PLATFORM_WINDOWS)
        if (strerror_s(errorString.data(), errorString.size(), errno))
        {
            return "Unknown error";
        }
        return {errorString.data()};
#else
        return strerror_r(errno, errorString.data(), errorString.size());
#endif
    }

    std::string
    GetExceptionStr(std::exception_ptr const& exc)
    {
        std::string excStr;
        if (!exc)
        {
            return excStr;
        }

        try
        {
            std::rethrow_exception(exc);
        }
        catch (std::exception const& e)
        {
            excStr = e.what();
        }
        catch (...)
        {
            excStr = "unknown";
        }
        return excStr;
    }

    std::string
    GetLastExceptionStr()
    {
        return GetExceptionStr(std::current_exception());
    }
} // namespace cppmicroservices::util
