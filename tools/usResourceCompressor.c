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

#define MINIZ_NO_TIME
#define MINIZ_NO_ARCHIVE_APIS
#include "miniz.c"

#include <limits.h>
#include <stdio.h>

typedef unsigned char uint8;
typedef unsigned int uint;

#define COMPRESS_MSG_BUFFER_SIZE 1024
static char us_compress_error[COMPRESS_MSG_BUFFER_SIZE];

#define my_max(a,b) (((a) > (b)) ? (a) : (b))
#define my_min(a,b) (((a) < (b)) ? (a) : (b))

#define BUF_SIZE (1024 * 1024)
static uint8 s_inbuf[BUF_SIZE];


const char* us_resource_compressor_error()
{
  return us_compress_error;
}

unsigned char* us_resource_compressor(FILE* pInfile, long file_loc, int level, long* out_size)
{
  const uint infile_size = (uint)file_loc;
  z_stream stream;
  uint infile_remaining = infile_size;
  long bytes_written = 0;
  unsigned char* s_outbuf = NULL;

  memset(us_compress_error, 0, COMPRESS_MSG_BUFFER_SIZE);

  if (file_loc < 0 || file_loc > INT_MAX)
  {
    sprintf(us_compress_error, "Resource too large to be processed.");
    return NULL;
  }

  s_outbuf = (unsigned char*)malloc(sizeof(unsigned char)*(infile_size+4));
  if (s_outbuf == NULL)
  {
    sprintf(us_compress_error, "Failed to allocate %d bytes for compression buffer.", infile_size);
    return NULL;
  }

  // Init the z_stream
  memset(&stream, 0, sizeof(stream));
  stream.next_in = s_inbuf;
  stream.avail_in = 0;
  stream.next_out = s_outbuf+4;
  stream.avail_out = infile_size;

  // Compression.
  if (deflateInit2(&stream, level, MZ_DEFLATED, -MZ_DEFAULT_WINDOW_BITS, 9, MZ_DEFAULT_STRATEGY) != Z_OK)
  {
    sprintf(us_compress_error, "deflateInit() failed.");
    free(s_outbuf);
    return NULL;
  }

  // Write the uncompressed file size in the first four bytes
  s_outbuf[0] = (unsigned char)((file_loc & 0xff000000) >> 24);
  s_outbuf[1] = (unsigned char)((file_loc & 0x00ff0000) >> 16);
  s_outbuf[2] = (unsigned char)((file_loc & 0x0000ff00) >> 8);
  s_outbuf[3] = (unsigned char)(file_loc & 0x000000ff);

  bytes_written = 4;
  for ( ; ; )
  {
    int status;
    if (!stream.avail_in)
    {
      // Input buffer is empty, so read more bytes from input file.
      uint n = my_min(BUF_SIZE, infile_remaining);

      if (fread(s_inbuf, 1, n, pInfile) != n)
      {
        sprintf(us_compress_error, "Failed reading from input file.");
        free(s_outbuf);
        return NULL;
      }

      stream.next_in = s_inbuf;
      stream.avail_in = n;

      infile_remaining -= n;
    }

    status = deflate(&stream, infile_remaining ? Z_NO_FLUSH : Z_FINISH);

    if ((status == Z_STREAM_END) || (!stream.avail_out))
    {
      // Output buffer is full, or compression is done.
      bytes_written += infile_size - stream.avail_out;
      break;
    }
    else if (status != Z_OK)
    {
      sprintf(us_compress_error, "deflate() failed with status %i.", status);
      free(s_outbuf);
      return NULL;
    }
  }

  if (deflateEnd(&stream) != Z_OK)
  {
    sprintf(us_compress_error, "deflateEnd() failed.");
    free(s_outbuf);
    return NULL;
  }

  if (out_size != NULL)
  {
    *out_size = bytes_written;
  }
  return s_outbuf;
}
