/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef USMODULERESOURCEBUFFER_P_H
#define USMODULERESOURCEBUFFER_P_H

#include <usConfig.h>

#include <streambuf>

US_BEGIN_NAMESPACE

class ModuleResourceBufferPrivate;

class US_EXPORT ModuleResourceBuffer: public std::streambuf
{

public:

  explicit ModuleResourceBuffer(const unsigned char* data, std::size_t size,
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
