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

#include <iostream>

#include "civetweb/civetweb.h"

#include "TestEnvironment.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/ServiceProperties.h"
#include <chrono>
#include <string>

// TestEnvironment* TestEnvironment::m_Instance = nullptr;

TestEnvironment::TestEnvironment(const std::string& httpPort)
{

  m_HttpPort = atoi(httpPort.c_str());

  cppmicroservices::FrameworkFactory frameworkFactory;

  const cppmicroservices::Framework fm = frameworkFactory.NewFramework();
  m_Framework = std::make_shared<cppmicroservices::Framework>(fm);

  m_Framework->Start();
  cppmicroservices::BundleContext context = m_Framework->GetBundleContext();

  m_Servlet.reset(new TestServlet());
  cppmicroservices::ServiceProperties props;
  std::string root = "/test";
  props[cppmicroservices::HttpServlet::PROP_CONTEXT_ROOT] = root;
  m_Registration =
    context.RegisterService<cppmicroservices::HttpServlet>(m_Servlet, props);

  m_Container = new cppmicroservices::ServletContainer(context);

  std::vector<std::string> options;
  options.push_back("listening_ports");
  options.push_back(httpPort);
  options.push_back("num_threads");
  options.push_back("2");
  m_Container->Start(options);

  std::cerr << "[INFO     ] Framework, container and servlet ready at port "
            << m_HttpPort << std::endl;
}

TestEnvironment::~TestEnvironment()
{
  m_Registration.Unregister();
  m_Container->Stop();
  m_Framework->WaitForStop(std::chrono::milliseconds(500));
  delete m_Container;

  std::cerr << "[INFO     ] Framework shut down" << std::endl;
}

std::shared_ptr<cppmicroservices::HttpServlet> TestEnvironment::GetServlet()
{
  return m_Servlet;
}

void TestEnvironment::HttpSend(const char* content,
                               std::ifstream::pos_type size)
{
#ifndef NDEBUG
  std::cerr << "[INFO      ] enter HttpSend(char *)" << std::endl;
#endif

  char error_buffer[1024];
  struct mg_connection* cnx;
  cnx = mg_connect_client(
    "127.0.0.1", m_HttpPort, 0, error_buffer, sizeof(error_buffer));
  if (NULL == cnx || strcmp("", error_buffer) > 0) {

#ifndef NDEBUG
    std::cerr << "[ERROR     ] " << error_buffer << std::endl;
#endif

  } else {
#ifndef NDEBUG
    std::cerr << "[INFO      ] Connection established (" << cnx << ")"
              << std::endl;
#endif
    mg_write(cnx, content, size);
    char client_err[1024];
    memset(client_err, 0, sizeof(client_err));

#ifndef NDEBUG
    int client_res = mg_get_response(cnx, client_err, sizeof(client_err), 1000);
    if (client_res < 0) {
      std::cerr << "[ERROR     ] '" << client_err << "'" << std::endl;
    } else {
      std::cerr << std::endl
                << "[INFO      ] '" << client_err << "'" << std::endl;
    }
#else
    mg_get_response(cnx, client_err, sizeof(client_err), 1000);
#endif

#ifndef NDEBUG
    std::cerr << "[INFO      ] exit HttpSend()" << std::endl;
#endif
  }
  auto responseInfo = mg_get_response_info(cnx);

  if (responseInfo != nullptr) {
    m_ResponseStatusText = std::string(responseInfo->status_text);
    m_ResponseHttpVersion = std::string(responseInfo->http_version);
  } else {
    std::cerr << "[WARNING      ] No HTTP response info available" << std::endl;
  }
  mg_close_connection(cnx);
}

void TestEnvironment::HttpSend(const std::string& content)
{
  std::ifstream::pos_type size = content.length();
  TestEnvironment::HttpSend(content.c_str(), size);
}

std::string TestEnvironment::getStatusText()
{
  return m_ResponseStatusText;
}

std::string TestEnvironment::getHttpVersion()
{
  return m_ResponseHttpVersion;
}
