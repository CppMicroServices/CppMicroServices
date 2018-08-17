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

#include "TestingConfig.h"
#include "gtest/gtest.h"
#include <civetweb/civetweb.h>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>

#include "cppmicroservices/httpservice/HttpServiceFactory.h"

namespace us = cppmicroservices;

//
// see https://github.com/civetweb/civetweb/blob/master/docs/api/mg_request_info.md
//

/*!
* We just have to mimic the basis of CivetWeb mg_connection to mock it.
* All support of the struct is not available, but what is available is 
* sufficient for most of the tests
*/
struct mg_connection
{
  int connection_type; /* see CONNECTION_TYPE_* above */

  struct mg_request_info request_info;
  struct mg_response_info response_info;

  struct mg_context* phys_ctx;
  struct mg_domain_context* dom_ctx;
};

enum
{
  CONNECTION_TYPE_INVALID,
  CONNECTION_TYPE_REQUEST,
  CONNECTION_TYPE_RESPONSE
};

/*!
 * Contains expected content when testing a HttpServletRequest
 *
 * Implement helper methods for query string and connection setup using
 * information it contains.
 */
struct Expected
{
  virtual ~Expected() {}

  std::string m_Scheme = "NOT SET";
  std::string m_ServerName = "NOT SET";
  std::string m_Path = "";
  int m_ServerPort = -1;
  std::map<std::string, us::Any> m_Parameters;
  std::map<std::string, std::string> m_HeadersAsString;
  std::vector<mg_header> m_Headers;
  std::string m_QueryString = "";
  std::string m_RequestUri = "";

  void CheckRequestContent(std::shared_ptr<us::HttpServletRequest> request)
  {
    std::string actualSscheme = request->GetScheme();
    std::string actualServerName = request->GetServerName();
    int actualServerPort = request->GetServerPort();

    ASSERT_EQ(m_Scheme, actualSscheme) << "Scheme must be available";

    ASSERT_EQ(m_ServerName, actualServerName)
      << "Server hostname must be available";

    ASSERT_EQ(m_ServerPort, actualServerPort) << "Param2 must be available";

    for (auto const& entry : m_Parameters) {
      std::string expectedKey = entry.first;
      us::Any expectedValue = entry.second;

      us::Any actualValue = request->GetParameter(expectedKey);

      ASSERT_FALSE(actualValue.Empty())
        << "Expected parameter " << expectedKey << " must not be empty";

      ASSERT_EQ(expectedValue.ToString(), actualValue.ToString())
        << "Request parameter " << expectedKey
        << " must be present with expected value";
    }
  }

  mg_header CreateHeader(const char* name, const char* value)
  {
    return mg_header({ name, value });
  }

  /*!
   * Setup a mg_connection using information it contains
   */
  void SetupConnection(std::shared_ptr<mg_connection> cnx)
  {
    m_RequestUri += m_Scheme;
    m_RequestUri += "://";
    m_RequestUri += m_ServerName;
    if (m_ServerPort > 0) {
      m_RequestUri += ":" + std::to_string(m_ServerPort);
    } else {
      m_ServerPort = 80; // default value in HttpServletRequest
    }

    if (m_Parameters.size() > 0) {
      m_QueryString += "?";
      for (const std::pair<std::string, us::Any> aParam : m_Parameters) {
        m_QueryString += aParam.first + "=" + aParam.second.ToString() + "&";
      }
    }
    std::cout << std::endl << "Request URI: " << m_RequestUri << std::endl;
    std::cout << std::endl
              << "Request query string: " << m_QueryString << std::endl;

    cnx->request_info.request_uri = m_QueryString.c_str();
    cnx->request_info.query_string = m_QueryString.c_str();

    // HTTP headers
    const std::string headerKey = "Host";
    const std::string headerValueAsString =
      m_Scheme + "://" + m_ServerName + ":" + std::to_string(m_ServerPort);
    m_HeadersAsString[headerKey] = headerValueAsString;

    std::cout << std::endl
              << "HTTP 'Host' header value: " << headerValueAsString
              << std::endl;

    size_t headerIndex = 0;
    std::map<std::string, std::string>::iterator paramIt;

    // to avoid compiler warning int <- size_t
    // below in 
    // cnx->request_info.num_headers = integerCounter;
    int integerCounter = 0;

    for (std::pair<const std::string&, const std::string&> const& header :
         m_HeadersAsString) {
      cnx->request_info.http_headers[headerIndex] =
        CreateHeader(header.first.c_str(), header.second.c_str());
      headerIndex++;
      integerCounter++;
    }
    cnx->request_info.num_headers = integerCounter;
  }
};

std::shared_ptr<mg_header> serverHeader;

std::shared_ptr<mg_connection> CreateMockConnection()
{
  std::shared_ptr<mg_connection> mock_connection =
    std::make_shared<mg_connection>();
  mock_connection->connection_type = CONNECTION_TYPE_REQUEST;

  return mock_connection;
}

TEST(HttpServletTest, Test_GetQueryStringParsingTwoParams)
{
  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "my.dummy.host1";
  EXPECTED.m_ServerPort = 8080;
  EXPECTED.m_Parameters["param11"] = us::Any(std::string("a"));
  EXPECTED.m_Parameters["param12"] = us::Any(std::string("b"));

  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetQueryStringParsingOneParam)
{
  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "my.dummy.host2";
  EXPECTED.m_ServerPort = 8080;
  EXPECTED.m_Parameters["param21"] = us::Any(std::string("a"));
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetQueryStringParsingNoParams)
{
  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "http";
  EXPECTED.m_ServerName = "my.dummy.host3";
  EXPECTED.m_ServerPort = 80; // default value
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetQueryStringParsingRootWithPort)
{
  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "my.dummy.host4";
  EXPECTED.m_ServerPort = 8080; // default value
  EXPECTED.SetupConnection(mockConnection);
  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetQueryStringParsingRootNoPort)
{
  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "my.dummy.host5";
  EXPECTED.m_ServerPort = 80; // default value
  EXPECTED.SetupConnection(mockConnection);
  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetQueryStringParsingNoPortUri)
{
  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "my.dummy.host7";
  EXPECTED.m_Parameters["param31"] = us::Any(std::string("a"));
  EXPECTED.m_Parameters["param32"] = us::Any(std::string("b"));
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetQueryStringParsingNoHost)
{
  auto mockConnection = CreateMockConnection();

  // no host
  //mockConnection->request_info.request_uri = "/toto?param51=a&param52=b";

  Expected EXPECTED;
  EXPECTED.m_Scheme = "http"; // default value
  EXPECTED.m_ServerPort = 80; // default value
  EXPECTED.m_Parameters["param51"] = us::Any(std::string("a"));
  EXPECTED.m_Parameters["param52"] = us::Any(std::string("b"));
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  EXPECTED.CheckRequestContent(request);
}

TEST(HttpServletTest, Test_GetServerPort)
{

  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "GetServerPort.test";
  EXPECTED.m_ServerPort = 8080; // default value
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  int actualServerPort = request->GetServerPort();

  ASSERT_EQ(EXPECTED.m_ServerPort, actualServerPort)
    << "Request port must be parsed";
}

TEST(HttpServletTest, Test_GetServerHost)
{

  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "GetServerHost.test";
  EXPECTED.m_ServerPort = 8080; // default value
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  std::string actualServerName = request->GetServerName();

  ASSERT_EQ(EXPECTED.m_ServerName, actualServerName)
    << "request host must be parsed";
}

TEST(HttpServletTest, Test_GetServerScheme)
{

  auto mockConnection = CreateMockConnection();

  Expected EXPECTED;
  EXPECTED.m_Scheme = "https";
  EXPECTED.m_ServerName = "GetServerScheme.test";
  EXPECTED.m_ServerPort = 8080; // default value
  EXPECTED.SetupConnection(mockConnection);

  auto request = cppmicroservices::HttpServiceFactory::CreateHttpServletRequest(
    nullptr, nullptr, mockConnection.get());

  std::string actualScheme = request->GetScheme();

  ASSERT_EQ(EXPECTED.m_Scheme, actualScheme) << "Request scheme must be parsed";
}
