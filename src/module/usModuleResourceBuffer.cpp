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

#include "usModuleResourceBuffer_p.h"

#include "stdint_p.h"

#include <limits>
#include <cassert>

#ifdef US_PLATFORM_WINDOWS
#define DATA_NEEDS_NEWLINE_CONVERSION 1
#undef REMOVE_LAST_NEWLINE_IN_TEXT_MODE
#else
#undef DATA_NEEDS_NEWLINE_CONVERSION
#define REMOVE_LAST_NEWLINE_IN_TEXT_MODE 1
#endif

US_BEGIN_NAMESPACE

static const std::size_t BUFFER_SIZE = 1024;

class ModuleResourceBufferPrivate
{
public:

  ModuleResourceBufferPrivate(const char* begin, std::size_t size, std::ios_base::openmode mode)
    : begin(begin)
    , end(begin + size)
    , current(begin)
    , mode(mode)
  #ifdef DATA_NEEDS_NEWLINE_CONVERSION
    , pos(0)
  #endif
  {
  }

  const char* const begin;
  const char* const end;
  const char* current;

  const std::ios_base::openmode mode;

#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  // records the stream position ignoring CR characters
  std::streambuf::pos_type pos;
#endif

};

ModuleResourceBuffer::ModuleResourceBuffer(const unsigned char* data, std::size_t _size,
                                           std::ios_base::openmode mode)
  : d(NULL)
{
  assert(_size < static_cast<std::size_t>(std::numeric_limits<uint32_t>::max()));
  assert(data != NULL);

  const char* begin = reinterpret_cast<const char*>(data);
  std::size_t size = _size;

#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  if (!(mode & std::ios_base::binary) && begin[0] == '\r')
  {
    ++begin;
    --size;
  }
#endif

#ifdef REMOVE_LAST_NEWLINE_IN_TEXT_MODE
  if(!(mode & std::ios_base::binary) && begin[size-1] == '\n')
  {
    --size;
  }
#endif

  d = new ModuleResourceBufferPrivate(begin, size, mode);
}

ModuleResourceBuffer::~ModuleResourceBuffer()
{
  delete d;
}

ModuleResourceBuffer::int_type ModuleResourceBuffer::underflow()
{
  if (d->current == d->end)
    return traits_type::eof();

#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  char c = *d->current;
  if (!(d->mode & std::ios_base::binary))
  {
    if (c == '\r')
    {
      if (d->current + 1 == d->end)
      {
        return traits_type::eof();
      }
      c = d->current[1];
    }
  }
  return traits_type::to_int_type(c);
#else
  return traits_type::to_int_type(*d->current);
#endif
}

ModuleResourceBuffer::int_type ModuleResourceBuffer::uflow()
{
  if (d->current == d->end)
    return traits_type::eof();

#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  char c = *d->current++;
  if (!(d->mode & std::ios_base::binary))
  {
    if (c == '\r')
    {
      if (d->current == d->end)
      {
        return traits_type::eof();
      }
      c = *d->current++;
    }
  }
  return traits_type::to_int_type(c);
#else
  return traits_type::to_int_type(*d->current++);
#endif
}

ModuleResourceBuffer::int_type ModuleResourceBuffer::pbackfail(int_type ch)
{
  int backOffset = -1;
#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  if (!(d->mode & std::ios_base::binary))
  {
    while ((d->current - backOffset) >= d->begin && d->current[backOffset] == '\r')
    {
      --backOffset;
    }
    // d->begin always points to a character != '\r'
  }
#endif
  if (d->current == d->begin || (ch != traits_type::eof() && ch != d->current[backOffset]))
  {
    return traits_type::eof();
  }

  d->current += backOffset;
  return traits_type::to_int_type(*d->current);
}

std::streamsize ModuleResourceBuffer::showmanyc()
{
  assert(std::less_equal<const char *>()(d->current, d->end));

#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  std::streamsize ssize = 0;
  std::size_t chunkSize = d->end - d->current;
  for (std::size_t i = 0; i < chunkSize; ++i)
  {
    if (d->current[i] != '\r')
    {
      ++ssize;
    }
  }
  return ssize;
#else
  return d->end - d->current;
#endif
}

std::streambuf::pos_type ModuleResourceBuffer::seekoff(std::streambuf::off_type off,
                                                       std::ios_base::seekdir way,
                                                       std::ios_base::openmode /*which*/)
{
#ifdef DATA_NEEDS_NEWLINE_CONVERSION
  std::streambuf::off_type step = 1;
  if (way == std::ios_base::beg)
  {
    d->current = d->begin;
  }
  else if (way == std::ios_base::end)
  {
    d->current = d->end;
    step = -1;
  }

  if (!(d->mode & std::ios_base::binary))
  {
    if (way == std::ios_base::beg)
    {
      d->pos = 0;
    }
    else if (way == std::ios_base::end)
    {
      d->current -= 1;
    }

    std::streambuf::off_type i = 0;
    // scan through off amount of characters excluding '\r'
    while (i != off)
    {
      if (*d->current != '\r')
      {
        i += step;
        d->pos += step;
      }
      d->current += step;
    }

    // adjust the position in case of a "backwards" seek
    if (way == std::ios_base::end)
    {
      // fix pointer from previous while loop
      d->current += 1;
      d->pos = 0;
      i = 0;
      const std::streampos currInternalPos = d->current - d->begin;
      while (i != currInternalPos)
      {
        if (d->begin[i] != '\r')
        {
          d->pos += 1;
        }
        ++i;
      }
    }
  }
  else
  {
    d->current += off;
    d->pos = d->current - d->begin;
  }
  return d->pos;
#else
  if (way == std::ios_base::beg)
  {
    d->current = d->begin + off;
    return off;
  }
  else if (way == std::ios_base::cur)
  {
    d->current += off;
    return d->current - d->begin;
  }
  else
  {
    d->current = d->end + off;
    return d->current - d->begin;
  }
#endif
}

std::streambuf::pos_type ModuleResourceBuffer::seekpos(std::streambuf::pos_type sp,
                                                       std::ios_base::openmode /*which*/)
{
  return this->seekoff(sp, std::ios_base::beg);
}

US_END_NAMESPACE
