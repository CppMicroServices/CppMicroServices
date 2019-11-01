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
#include <fstream>
#include <iostream>
#include <memory>

#include "TestEnvironment.h"
#include "cppmicroservices/httpservice/HttpServiceFactory.h"

#include <civetweb/civetweb.h>

namespace us = cppmicroservices;

TEST(HttpServletTest, Test_GetSetStatus)
{
  auto response =
    cppmicroservices::HttpServiceFactory::CreateHttpServletResponse(
      nullptr, nullptr, nullptr);

  response->SetStatus(200);
  int status = response->GetStatus();
  ASSERT_EQ(200, status) << "status code must be setable and getable";
}

TEST(HttpServletTest, Test_GetSetEncoding)
{
  auto response =
    cppmicroservices::HttpServiceFactory::CreateHttpServletResponse(
      nullptr, nullptr, nullptr);

  response->SetCharacterEncoding("utf-8");
  std::string encoding = response->GetCharacterEncoding();
  ASSERT_EQ("utf-8", encoding) << "encoding must be setable and getable";
}

TEST(HttpServletTest, Test_GetSetContentType)
{
  auto response =
    cppmicroservices::HttpServiceFactory::CreateHttpServletResponse(
      nullptr, nullptr, nullptr);

  response->SetContentType("text/plain");
  std::string encoding = response->GetContentType();
  ASSERT_EQ(encoding, "text/plain; UTF-8")
    << "content type must be setable and getable";

  response->SetCharacterEncoding("latin-1");
  encoding = response->GetContentType();
  ASSERT_EQ(encoding, "text/plain; latin-1")
    << "content type must be setable and getable";
}

TEST(HttpServletTest, Test_HeadersManipulation)
{
  auto response =
    cppmicroservices::HttpServiceFactory::CreateHttpServletResponse(
      nullptr, nullptr, nullptr);

  ASSERT_FALSE(response->ContainsHeader("non existing Header"))
    << "'non existing Header' header does not exists";
  ASSERT_THROW(response->GetHeader("non existing Header"), std::runtime_error)
    << "Getting unknown header must throw";

  response->AddHeader("testName", "testValue");
  ASSERT_TRUE(response->ContainsHeader("testName"))
    << "'testName' header has just beed added";
  ASSERT_NO_THROW(response->GetHeader("testName"))
    << "Getting existing header must not throw";
  ASSERT_EQ(response->GetHeader("testName"), "testValue")
    << "Getting existing header must return correct value";

  response->SetHeader("testName2", "testValue2");
  ASSERT_TRUE(response->ContainsHeader("testName2"))
    << "'testName2' header has just beed added";
  ASSERT_NO_THROW(response->GetHeader("testName2"))
    << "Getting existing header must not throw";
  ASSERT_EQ(response->GetHeader("testName2"), "testValue2")
    << "Getting existing header must return correct value";

  response->AddIntHeader("testIntName", 42);
  ASSERT_TRUE(response->ContainsHeader("testIntName"))
    << "'testIntName' header has just beed added";
  ASSERT_NO_THROW(response->GetHeader("testIntName"))
    << "Getting existing header must not throw";
  ASSERT_EQ(response->GetHeader("testIntName"), "42")
    << "Getting existing header must return correct value";

  response->SetIntHeader("testIntName2", 43);
  ASSERT_TRUE(response->ContainsHeader("testIntName2"))
    << "'testIntName2' header has just beed added";
  ASSERT_NO_THROW(response->GetHeader("testIntName2"))
    << "Getting existing header must not throw";
  ASSERT_EQ(response->GetHeader("testIntName2"), "43")
    << "Getting existing header must return correct value";
}
