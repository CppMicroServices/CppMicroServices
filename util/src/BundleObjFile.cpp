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

#include "cppmicroservices/util/BundleObjFile.h"

#include <cstring>
#include <utility>
#include <string>
#include <array>

US_MSVC_PUSH_DISABLE_WARNING(4996)

namespace cppmicroservices
{
  namespace
    {
        constexpr std::size_t ErrorBufferSize = 256;
    }

    InvalidObjFileException::InvalidObjFileException(std::string what, int errorNumber) : m_What(std::move(what))
    {
        if (errorNumber)
        {
            std::array<char, ErrorBufferSize> errBuf{};

#ifdef US_PLATFORM_WINDOWS
            strerror_s(errBuf.data(), errBuf.size(), errorNumber);
#else
            strerror_r(errorNumber, errBuf.data(), errBuf.size()); // POSIX version
#endif

            m_What += ": ";
            m_What += errBuf.data(); // Safe conversion
        }
    }

    char const*
    InvalidObjFileException::what() const noexcept
    {
        return m_What.c_str();
    }
} // namespace cppmicroservices

US_MSVC_POP_WARNING
