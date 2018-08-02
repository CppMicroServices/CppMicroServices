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

#include "HttpOutputStreamBuffer.h"

#include "HttpServletResponsePrivate.h"

#include "civetweb/civetweb.h"

#include <cassert>
#include <functional>

namespace cppmicroservices {

HttpOutputStreamBuffer::HttpOutputStreamBuffer(
  HttpServletResponsePrivate* response,
  std::size_t bufferSize)
  : m_Buffer(bufferSize + 1)
  , m_Response(response)
  , m_ChunkedCoding(true)
{
  char* base = &m_Buffer.front();
  setp(base, base + m_Buffer.size() - 1);
}

HttpOutputStreamBuffer::~HttpOutputStreamBuffer()
{
  if (m_Response->m_Connection == nullptr)
    return;

  // if not committed yet and (i.e. the content completely fits
  // into the buffer and nobody synced the stream yet), then
  // set the Content-Length if it is not automatically set.
  // This is equal to an explicit "close" operation.
  if (!m_Response->m_IsCommited &&
      m_Response->m_Headers.find("Content-Length") !=
        m_Response->m_Headers.end()) {
    m_Response->m_Headers["Content-Length"] =
      m_Response->LexicalCast(static_cast<long>(pptr() - pbase()));
  }
  sync();
  if (m_ChunkedCoding) {
    mg_write(m_Response->m_Connection, "0\r\n\r\n", 5);
  }
}

bool HttpOutputStreamBuffer::CommitStream()
{
  if (!m_Response->m_IsCommited) {
    m_ChunkedCoding = m_Response->m_Headers.find("Content-Length") ==
                      m_Response->m_Headers.end();
    if (m_ChunkedCoding) {
      m_Response->m_Headers["Transfer-Encoding"] = "chunked";
    }
    // this writes the headers if not already written
    return m_Response->Commit();
  }
  return true;
}

std::streambuf::int_type HttpOutputStreamBuffer::overflow(int_type ch)
{
  if (ch != traits_type::eof()) {
    assert(std::less_equal<char*>()(pptr(), epptr()));
    *pptr() = ch;
    pbump(1);
    if (sendBuffer()) {
      return ch;
    }
  }
  return traits_type::eof();
}

int HttpOutputStreamBuffer::sync()
{
  return sendBuffer() ? 0 : -1;
}

bool HttpOutputStreamBuffer::sendBuffer()
{
  if (!m_Response->m_Connection)
    return false;

  // commit the response by writing the headers
  if (!CommitStream())
    return false;

  // write the buffer contents
  std::ptrdiff_t n = pptr() - pbase();
  pbump(static_cast<int>(-n));
  int bytesSend = 0;
  if (m_ChunkedCoding) {
    // send chunk size
    std::string chunkSize =
      m_Response->LexicalCastHex(static_cast<long>(n)) + "\r\n";
    bytesSend +=
      mg_write(m_Response->m_Connection, &chunkSize[0], chunkSize.size());
  }
  bytesSend += mg_write(m_Response->m_Connection, pbase(), n);
  if (m_ChunkedCoding) {
    bytesSend += mg_write(m_Response->m_Connection, "\r\n", 2);
  }
  return bytesSend > 0;
}
}
