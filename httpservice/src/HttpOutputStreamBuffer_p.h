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

#ifndef HTTPOUTPUTSTREAMBUFFER_H
#define HTTPOUTPUTSTREAMBUFFER_H

#include "cppmicroservices/GlobalConfig.h"

#include <streambuf>
#include <vector>

namespace cppmicroservices {

struct HttpServletResponsePrivate;

class HttpOutputStreamBuffer : public std::streambuf
{
public:
  explicit HttpOutputStreamBuffer(HttpServletResponsePrivate* response, std::size_t bufferSize = 1024);
  ~HttpOutputStreamBuffer();

protected:

  bool CommitStream();

private:

  int_type overflow(int_type ch);

  int sync();

  bool sendBuffer();

  HttpOutputStreamBuffer(const HttpOutputStreamBuffer&);
  HttpOutputStreamBuffer& operator=(const HttpOutputStreamBuffer&);

private:

  std::vector<char> m_Buffer;
  HttpServletResponsePrivate* m_Response;
  bool m_ChunkedCoding;
};

}

#endif // HTTPOUTPUTSTREAMBUFFER_H
