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

#include "usUncompressResourceData.h"

#ifdef US_ENABLE_RESOURCE_COMPRESSION
#include "usLog_p.h"

extern "C" {
const char* us_uncompress_resource_error();
void us_uncompress_resource_data(const unsigned char*, size_t, unsigned char*, size_t);
}

US_BEGIN_NAMESPACE

unsigned char* UncompressResourceData(const unsigned char* data, std::size_t size,
                                      std::size_t* uncompressedSize)
{
  if (size <= 4)
  {
    if (size < 4 || (data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 0))
    {
      US_WARN << "UncompressResourceData: Input data is corrupted";
      return NULL;
    }
  }
  std::size_t expectedSize = (data[0] << 24) | (data[1] << 16) | (data[2] <<  8) | data[3];
  try
  {
    unsigned char* uncompressedData = new unsigned char[expectedSize];
    us_uncompress_resource_data(data+4, size-4, uncompressedData, expectedSize);
    if (us_uncompress_resource_error()[0] != 0)
    {
      US_WARN << us_uncompress_resource_error();
      delete[] uncompressedData;
      return NULL;
    }

    if (uncompressedSize != NULL)
    {
      *uncompressedSize = expectedSize;
    }

    return uncompressedData;
  }
  catch (const std::bad_alloc&)
  {
    US_WARN << "UncompressResourceData: Could not allocate enough memory for uncompressing resource data";
    return NULL;
  }

  return NULL;
}

US_END_NAMESPACE

#else

US_BEGIN_NAMESPACE

unsigned char* UncompressResourceData(const unsigned char*, std::size_t, std::size_t*)
{
  return 0;
}

US_END_NAMESPACE

#endif // US_ENABLE_RESOURCE_COMPRESSION
