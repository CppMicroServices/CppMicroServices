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

#include "cppmicroservices/httpservice/HttpServletPart.h"

#include "FileHttpServletPartPrivate.h"
#include "HttpServletPartPrivate.h"
#include "MemoryHttpServletPartPrivate.h"

#include "cppmicroservices/httpservice/ServletContext.h"

#include "civetweb/civetweb.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

namespace cppmicroservices {

FileHttpServletPartPrivate::FileHttpServletPartPrivate(
  const std::shared_ptr<ServletContext>& servletContext,
  CivetServer* server,
  mg_connection* conn)
  : HttpServletPartPrivate(servletContext, server, conn)
{}

std::istream* FileHttpServletPartPrivate::GetInputStream() const
{
  std::istream* is = new std::ifstream(m_TemporaryFileName, std::ios::binary);
  return is;
}

void FileHttpServletPartPrivate::Delete()
{
  const char* filename = m_TemporaryFileName.c_str();
  std::ifstream f(filename);
  if (f.good()) {
    f.close();
    int status = remove(filename);
    if (status != 0) {
      std::cerr << "Temporary file '" << m_TemporaryFileName
                << "' cannot be deleted" << std::endl;
    }
  }
}

std::string FileHttpServletPartPrivate::GetSubmittedFileName() const
{
  return m_SubmittedFileName;
}

long long FileHttpServletPartPrivate::GetSize() const
{
  return m_Size;
}

MemoryHttpServletPartPrivate::MemoryHttpServletPartPrivate(
  const std::shared_ptr<ServletContext>& servletContext,
  CivetServer* server,
  mg_connection* conn)
  : HttpServletPartPrivate(servletContext, server, conn)
{}

std::string MemoryHttpServletPartPrivate::GetSubmittedFileName() const
{
  return HttpServletPart::NO_FILE;
}

std::istream* MemoryHttpServletPartPrivate::GetInputStream() const
{
  std::istringstream* sb = new std::istringstream(m_Value);
  return sb;
}

void MemoryHttpServletPartPrivate::Delete()
{
  m_Value = "";
}

long long MemoryHttpServletPartPrivate::GetSize() const
{
  return m_Value.size();
}

HttpServletPartPrivate::HttpServletPartPrivate(
  const std::shared_ptr<ServletContext>& servletContext,
  CivetServer* server,
  mg_connection* conn)
  : m_ServletContext(servletContext)
  , m_Server(server)
  , m_Connection(conn)
{}

HttpServletPart::~HttpServletPart()
{
  this->Delete();
}

HttpServletPart::HttpServletPart(const HttpServletPart& o)
  : d(o.d)
{}

HttpServletPart& HttpServletPart::operator=(const HttpServletPart& o)
{
  d = o.d;
  return *this;
}

const std::string HttpServletPart::NO_FILE = "";

void HttpServletPart::Delete()
{
  d->Delete();
}

std::string HttpServletPart::GetContentType() const
{
  return mg_get_builtin_mime_type(GetSubmittedFileName().c_str());
}

std::string HttpServletPart::GetHeader(const std::string& /* name */) const
{
  return std::string();
}

std::vector<std::string> HttpServletPart::GetHeaderNames() const
{
  std::vector<std::string> headerNames;
  return headerNames;
}

std::vector<std::string> HttpServletPart::GetHeaders(
  const std::string& /* name */) const
{
  std::vector<std::string> headers;
  return headers;
}

std::istream* HttpServletPart::GetInputStream()
{
  return d->GetInputStream();
}

std::string HttpServletPart::GetName() const
{
  return d->m_Name;
}

long long HttpServletPart::GetSize() const
{
  return d->GetSize();
}

std::string HttpServletPart::GetSubmittedFileName() const
{
  return d->GetSubmittedFileName();
}

void HttpServletPart::Write(const std::string& fileName)
{
  std::ofstream newFile(fileName, std::ofstream::out);
  std::istream* temporaryFile = GetInputStream();
  char buffer[128];
  while (!temporaryFile->eof()) {
    std::streamsize bytesIn =
      temporaryFile->read(buffer, sizeof(buffer)).gcount();
    newFile.write(buffer, bytesIn);
  }
  newFile.close();
  delete temporaryFile;
}

HttpServletPart::HttpServletPart(HttpServletPartPrivate* d)
  : d(d)
{}
}
