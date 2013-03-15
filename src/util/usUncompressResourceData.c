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


#include "tinfl.c"

#include <stdio.h>

static char us_uncompress_error_msg[256];

const char* us_uncompress_resource_error()
{
  return us_uncompress_error_msg;
}

void us_uncompress_resource_data(const unsigned char* data, size_t size,
                                 unsigned char* uncompressedData, size_t uncompressedSize)
{
  size_t bytesWritten = 0;

  memset(us_uncompress_error_msg, 0, sizeof(us_uncompress_error_msg));

  if (data == NULL)
  {
    return;
  }

  if (uncompressedData == NULL)
  {
    sprintf(us_uncompress_error_msg, "us_uncompress_resource_data: Buffer for uncomcpressing data is NULL");
    return;
  }

  bytesWritten = tinfl_decompress_mem_to_mem(uncompressedData, uncompressedSize,
                                             data, size,
                                             TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  if (bytesWritten != uncompressedSize)
  {
    sprintf(us_uncompress_error_msg, "us_uncompress_resource_data: tinfl_decompress_mem_to_mem failed");
  }
}
