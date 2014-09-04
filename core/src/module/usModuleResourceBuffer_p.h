/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef USMODULERESOURCEBUFFER_P_H
#define USMODULERESOURCEBUFFER_P_H

#include <usCoreExport.h>

#include <streambuf>

US_BEGIN_NAMESPACE

class ModuleResourceBufferPrivate;

class US_Core_EXPORT ModuleResourceBuffer: public std::streambuf
{

public:

  explicit ModuleResourceBuffer(void* data, std::size_t size,
                                std::ios_base::openmode mode);

  ~ModuleResourceBuffer();

private:

  int_type underflow();

  int_type uflow();

  int_type pbackfail(int_type ch);

  std::streamsize showmanyc();

  pos_type seekoff (off_type off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
  pos_type seekpos (pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

  // purposely not implemented
  ModuleResourceBuffer(const ModuleResourceBuffer&);
  ModuleResourceBuffer& operator=(const ModuleResourceBuffer&);

private:

  ModuleResourceBufferPrivate* d;

};

US_END_NAMESPACE

#endif // USMODULERESOURCEBUFFER_P_H
