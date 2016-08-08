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

#ifndef BUNDLERESOURCEBUFFER_P_H
#define BUNDLERESOURCEBUFFER_P_H

#include "cppmicroservices/FrameworkExport.h"

#include <streambuf>

namespace cppmicroservices {

namespace detail {

class BundleResourceBufferPrivate;

class US_Framework_EXPORT BundleResourceBuffer: public std::streambuf
{

public:

  BundleResourceBuffer(const BundleResourceBuffer&) = delete;
  BundleResourceBuffer& operator=(const BundleResourceBuffer&) = delete;

  explicit BundleResourceBuffer(void* data, std::size_t size,
                                std::ios_base::openmode mode);

  ~BundleResourceBuffer();

private:

  int_type underflow();

  int_type uflow();

  int_type pbackfail(int_type ch);

  std::streamsize showmanyc();

  pos_type seekoff (off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
  pos_type seekpos (pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

private:

  BundleResourceBufferPrivate* d;

};

} // namespace detail

} // namespace cppmicroservices

#endif // BUNDLERESOURCEBUFFER_P_H
