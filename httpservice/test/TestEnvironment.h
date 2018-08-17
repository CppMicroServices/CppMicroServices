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
#include "civetweb/civetweb.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/ServiceRegistration.h"
#include "cppmicroservices/httpservice/HttpServlet.h"
#include "cppmicroservices/httpservice/ServletContainer.h"
#include "gtest/gtest.h"

#include <fstream>

class TestEnvironment
{
private:
  std::shared_ptr<cppmicroservices::HttpServlet> m_Servlet;
  cppmicroservices::ServletContainer* m_Container;
  std::shared_ptr<cppmicroservices::Framework> m_Framework;
  int m_HttpPort;
  cppmicroservices::ServiceRegistration<cppmicroservices::HttpServlet>
    m_Registration;

  std::string m_ResponseStatusText;
  std::string m_ResponseHttpVersion;

public:
  /*!
  * Backed server will listen http://lcoalhost:httpPort
  *
  * /!\ Pay attention to not reuse port numbers in a testsuite /!\
  *
  */
  TestEnvironment(const std::string& httpPort);

  virtual ~TestEnvironment();

  /*!
   * Return a pointer on the servlet listening on "127.0.0.1:m_HttpPort/test"
   */
  std::shared_ptr<cppmicroservices::HttpServlet> GetServlet();

  /*!
   * Post HTTP content on local http service, at 127.0.0.1:m_HttpPort
   */
  void HttpSend(const std::string& content);
  void HttpSend(const char* content, std::ifstream::pos_type size);

  std::string getHttpVersion();
  std::string getStatusText();
};
