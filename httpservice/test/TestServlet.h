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

#ifndef TEST_SERVLET_H
#define TEST_SERVLET_H

#include "cppmicroservices/httpservice/HttpServlet.h"
#include "cppmicroservices/httpservice/HttpServletRequest.h"
#include "cppmicroservices/httpservice/HttpServletResponse.h"

/*!
 * Helper servlet used for HttpService testing
 *
 * Implement http methods processing (POST, GET..) and set state variable
 * helping testing.
 */
class TestServlet : public cppmicroservices::HttpServlet
{
public:
  static const std::string GET_RESPONSE_OK;
  static const std::string POST_RESPONSE_OK;
  static const std::string PUT_RESPONSE_OK;
  static const std::string DELETE_RESPONSE_OK;

  TestServlet();
  virtual ~TestServlet();

  /*!
   * Return the last processed request, or null.
   */
  cppmicroservices::HttpServletRequest* GetProcessedRequest();

  /*!
  * return the last generated response
  */
  cppmicroservices::HttpServletResponse* GetGeneratedResponse();

  /*!
   * Return false until that servlet received an operation to perform
   */
  bool OperationReceived();

  void DoPost(cppmicroservices::HttpServletRequest& request,
              cppmicroservices::HttpServletResponse& response) override;

  void DoPut(cppmicroservices::HttpServletRequest& request,
             cppmicroservices::HttpServletResponse& response) override;

  void DoDelete(cppmicroservices::HttpServletRequest& request,
                cppmicroservices::HttpServletResponse& response) override;

  void DoGet(cppmicroservices::HttpServletRequest& request,
             cppmicroservices::HttpServletResponse& response) override;

  void SetRequestBody(const std::string& body);
  std::string GetRequestBody();

private:
  cppmicroservices::HttpServletRequest* m_Request;
  bool m_OperationReceived;
  size_t partCount;
  std::string m_RequestBody;

  /*!
  * dele the last processed request
  */
  void Destroy() override;
};

#endif
