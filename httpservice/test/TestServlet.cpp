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

#include "TestServlet.h"
#include <iostream>

TestServlet::TestServlet()
  : cppmicroservices::HttpServlet()
{
  m_Request = nullptr;
  m_OperationReceived = false;
  partCount = 0;
}

TestServlet::~TestServlet() {}

void TestServlet::DoPost(cppmicroservices::HttpServletRequest& request,
                         cppmicroservices::HttpServletResponse& response)
{

#ifndef NDEBUG
  std::cerr << "[INFO      ] "
            << "POST request received" << std::endl;
#endif

  this->m_OperationReceived = true;
  this->m_Request = new cppmicroservices::HttpServletRequest(request);

  request.ReadParts();

  response.GetOutputStream() << POST_RESPONSE_OK;
  response.SetContentType("text/plain");
  response.SetContentLength(GET_RESPONSE_OK.length());
  if (request.GetContentLength() > 0) {
    SetRequestBody(request.GetBody());
  }
  response.SetStatus(200);
}

void TestServlet::Destroy()
{
  if (nullptr != m_Request) {
    delete m_Request;
  }
  this->HttpServlet::Destroy();
}

void TestServlet::DoGet(cppmicroservices::HttpServletRequest& request,
                        cppmicroservices::HttpServletResponse& response)
{
#ifndef NDEBUG
  std::cerr << "[INFO      ] "
            << "GET request received" << std::endl;
#endif

  this->m_OperationReceived = true;

  this->m_Request = new cppmicroservices::HttpServletRequest(request);

  response.GetOutputStream() << GET_RESPONSE_OK;
  response.SetContentLength(GET_RESPONSE_OK.length());
  response.SetContentType("text/plain");
  response.SetStatus(200);
}

void TestServlet::DoPut(cppmicroservices::HttpServletRequest& request,
                        cppmicroservices::HttpServletResponse& response)
{
#ifndef NDEBUG
  std::cerr << "[INFO      ] "
            << "PUT request received" << std::endl;
#endif

  this->m_OperationReceived = true;

  this->m_Request = new cppmicroservices::HttpServletRequest(request);

  response.GetOutputStream() << PUT_RESPONSE_OK;
  response.SetContentLength(PUT_RESPONSE_OK.length());
  response.SetContentType("text/plain");

  if (request.GetContentLength() > 0) {
    SetRequestBody(request.GetBody());
  }
  response.SetStatus(200);
}

void TestServlet::SetRequestBody(const std::string& body)
{
  m_RequestBody = body;
}

std::string TestServlet::GetRequestBody()
{
  return m_RequestBody;
}

void TestServlet::DoDelete(cppmicroservices::HttpServletRequest& request,
                           cppmicroservices::HttpServletResponse& response)
{
#ifndef NDEBUG
  std::cerr << "[INFO      ] "
            << "DELETE request received" << std::endl;
#endif

  this->m_OperationReceived = true;

  this->m_Request = new cppmicroservices::HttpServletRequest(request);

  response.GetOutputStream() << DELETE_RESPONSE_OK;
  response.SetContentLength(DELETE_RESPONSE_OK.length());
  response.SetContentType("text/plain");
  response.SetStatus(200);
}

cppmicroservices::HttpServletRequest* TestServlet::GetProcessedRequest()
{
  return this->m_Request;
}

cppmicroservices::HttpServletResponse* TestServlet::GetGeneratedResponse()
{
  return nullptr; // this->m_Response;
}

bool TestServlet::OperationReceived()
{
  return this->m_OperationReceived;
}
const std::string TestServlet::GET_RESPONSE_OK = "GET received";

const std::string TestServlet::PUT_RESPONSE_OK = "PUT received";

const std::string TestServlet::DELETE_RESPONSE_OK = "DELETE received";

const std::string TestServlet::POST_RESPONSE_OK = "POST received";
