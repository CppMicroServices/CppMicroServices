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

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <istream>
#include <stdio.h>
#include <thread>

#include "TestEnvironment.h"
#include "TestHttpRequests.h"

#include "cppmicroservices/httpservice/HttpServletPart.h"
#include "cppmicroservices/util/FileSystem.h"

namespace us = cppmicroservices;

class HttpServletPartTest : public ::testing::Test
{
public:
  HttpServletPartTest()
  {
    // initialization code here
  }

  void SetUp() {}

  void TearDown()
  {
    // code here will be called just after the test completes
    // ok to through exceptions from here if need be
  }

  ~HttpServletPartTest()
  {
    // cleanup any pending stuff, but no exceptions allowed
  }

  // put in any custom data members that you need
};

TEST_F(HttpServletPartTest, GetContentType_oneFileMultipart)
{
  TestEnvironment testEnv("8080");

  testEnv.HttpSend(Queries::POST_MULTIPART_ONE_FILE_REQUEST);

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  us::HttpServletRequest* request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "POST operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  ASSERT_TRUE((request->GetParts().size() == 1))
    << "One file is transmitted in this request";

  us::IHttpServletPart* part = request->GetParts()[0];
  ASSERT_TRUE(part->GetContentType() == "text/plain")
    << "Part.GetContentType() returns the content-type sent by the request";
}

TEST_F(HttpServletPartTest, GetName_oneFileMultipart)
{
  TestEnvironment testEnv("8081");

  testEnv.HttpSend(Queries::POST_MULTIPART_ONE_FILE_REQUEST);

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  us::HttpServletRequest* request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "POST operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  ASSERT_TRUE((request->GetParts().size() == 1))
    << "One file is transmitted in this request";

  us::IHttpServletPart* part = request->GetParts()[0];
  ASSERT_TRUE(part->GetName() == "sampleFileFormName")
    << "Part.GetName() returns the name of the part";
}

TEST_F(HttpServletPartTest, GetSubmittedFilename_oneFileMultipart)
{
  TestEnvironment testEnv("8082");

  testEnv.HttpSend(Queries::POST_MULTIPART_ONE_FILE_REQUEST);

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  us::HttpServletRequest* request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "POST operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  ASSERT_TRUE((request->GetParts().size() == 1))
    << "One file is transmitted in this request";

  us::IHttpServletPart* part = request->GetParts()[0];
  ASSERT_TRUE(part->GetSubmittedFileName() == "sampleFile.txt")
    << "Part.GetSubmittedFilename() returns the filename sent by the request";
}

TEST_F(HttpServletPartTest, GetInputStream_multipart)
{
  TestEnvironment testEnv("8083");

  testEnv.HttpSend(Queries::POST_MULTIPART_ONE_FILE_REQUEST);

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  us::HttpServletRequest* request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "POST operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  ASSERT_TRUE((request->GetParts().size() == 1))
    << "One file is transmitted in this request";

  us::IHttpServletPart* myPart = request->GetPart("sampleFileFormName");

  std::istream* partValue = myPart->GetInputStream();
  std::string actualContent{ std::istreambuf_iterator<char>(*partValue),
                             std::istreambuf_iterator<char>() };

  delete partValue;

  std::string EXPECTED = "hello world\r\n";

  ASSERT_EQ(EXPECTED, actualContent)
    << "temporary file content must equals 'hello world\\r\\n' ";
}

TEST_F(HttpServletPartTest, Write_multipart)
{
  TestEnvironment testEnv("8084");

  testEnv.HttpSend(Queries::POST_MULTIPART_ONE_FILE_REQUEST);

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  us::HttpServletRequest* request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "POST operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  std::string temporaryDirname = us::util::MakeUniqueTempDirectory();
  std::string temporaryFilename =
    us::util::MakeUniqueTempFile(temporaryDirname).Path;

  ASSERT_TRUE((request->GetParts().size() == 1))
    << "One file is transmitted in this request";

  us::IHttpServletPart* myPart = request->GetPart("sampleFileFormName");

  myPart->Write(temporaryFilename);

  std::ifstream file(temporaryFilename);
  std::string actualContent((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

  file.close();

  std::string EXPECTED = "hello world\r\n";

  ASSERT_EQ(EXPECTED, actualContent)
    << "temporary file content must equals 'hello world\\r\\n' ";

  us::util::RemoveDirectoryRecursive(temporaryDirname);
}

TEST_F(HttpServletPartTest, GetName_POST_form_data)
{
  TestEnvironment testEnv("8085");

  testEnv.HttpSend(Queries::POST_FORM_DATA);
  const std::string EXPECTED_VALUE = "the A value";

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  auto request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "GET operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  size_t partCount = request->GetParts().size();
  size_t TWO = 2;
  ASSERT_EQ(TWO, partCount) << "We must have two parts in this request";

  ASSERT_TRUE(request->GetParts()[0]->GetName() == "A")
    << "Part A must have the name A";
  ASSERT_TRUE(request->GetParts()[1]->GetName() == "B")
    << "Part A must have the name A";
}

TEST_F(HttpServletPartTest, GetSubmittedName_POST_form_data)
{
  TestEnvironment testEnv("8086");

  testEnv.HttpSend(Queries::POST_FORM_DATA);
  const std::string EXPECTED_VALUE = "the A value";

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  auto request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "GET operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  size_t partCount = request->GetParts().size();
  size_t TWO = 2;
  ASSERT_EQ(TWO, partCount) << "We must have two parts in this request";

  ASSERT_TRUE(request->GetParts()[0]->GetSubmittedFileName() ==
              cppmicroservices::HttpServletPart::NO_FILE)
    << "Part A must have the name A";
  ASSERT_TRUE(request->GetParts()[1]->GetSubmittedFileName() ==
              cppmicroservices::HttpServletPart::NO_FILE)
    << "Part A must have the name A";
}

TEST_F(HttpServletPartTest, GetInputStream_POST_binary_multipart)
{
  const char* RAW_REQ_FILE = "resources/http_post_mini_png.raw";
  const char* EXPECTED_OUTPUT_FILE = "resources/mini.png";

  const size_t BUFF_SIZE = 1024;
  char baseFolder[BUFF_SIZE];
  char* result = getcwd(baseFolder, BUFF_SIZE);
  if (NULL != result) {

    std::string DIR_SEP = "/";

    std::string rawReqFile = baseFolder + DIR_SEP + RAW_REQ_FILE;
    std::string expectedOutputFile =
      baseFolder + DIR_SEP + EXPECTED_OUTPUT_FILE;

    std::ifstream binaryRequestFile(rawReqFile, std::ios::binary);
    std::ifstream expectedImage(expectedOutputFile, std::ios::binary);

    if (binaryRequestFile.good() && expectedImage.good()) {

      std::string binaryRequest{ std::istreambuf_iterator<char>(
                                   binaryRequestFile),
                                 std::istreambuf_iterator<char>() };

      TestEnvironment testEnv("8087");

      testEnv.HttpSend(binaryRequest);

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      TestServlet* servlet =
        dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

      ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";
      ASSERT_TRUE(servlet->OperationReceived())
        << "POST operation must have hit the servlet";

      us::HttpServletRequest* request = servlet->GetProcessedRequest();

      ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

      size_t partCount = request->GetParts().size();
      size_t ONE = 1;
      ASSERT_EQ(ONE, partCount) << "We must have one part in this request";

      std::istream* partValue = request->GetPart("file")->GetInputStream();

      std::string actualContent{ std::istreambuf_iterator<char>(*partValue),
                                 std::istreambuf_iterator<char>() };

      delete partValue;

      std::string EXPECTED{ std::istreambuf_iterator<char>(expectedImage),
                            std::istreambuf_iterator<char>() };

      expectedImage.close();

      ASSERT_EQ(EXPECTED, actualContent)
        << "temporary file content must equals '" << EXPECTED << "' ";
    } else {
      FAIL() << "Cannot found one of test asset files '" << rawReqFile
             << "' or '" << expectedOutputFile << "'" << std::endl;
    }
  } else {
    FAIL() << "Cannot read current directory" << std::endl;
  }
}

TEST_F(HttpServletPartTest, HeaderGetters)
{
  TestEnvironment testEnv("8088");

  testEnv.HttpSend(Queries::POST_FORM_DATA);

  TestServlet* servlet = dynamic_cast<TestServlet*>(testEnv.GetServlet().get());

  ASSERT_TRUE((nullptr != servlet)) << "Servlet must be created";

  auto request = servlet->GetProcessedRequest();

  ASSERT_TRUE(servlet->OperationReceived())
    << "GET operation must have hit the servlet";
  ASSERT_TRUE((nullptr != request)) << "Processed request must not be null";

  size_t partCount = request->GetParts().size();
  size_t TWO = 2;
  ASSERT_EQ(TWO, partCount) << "We must have two parts in this request";

  auto part0 = request->GetParts()[0];
  ASSERT_FALSE(nullptr == part0) << "There must be two parts in the request";
  auto part1 = request->GetParts()[1];
  ASSERT_FALSE(nullptr == part1) << "There must be two parts in the request";

  ASSERT_EQ("", part0->GetHeader("Content-Disposition"));
  ASSERT_EQ("", part0->GetHeader("Not yet implemented"));
  size_t ZERO = 0;
  ASSERT_EQ(ZERO, part1->GetHeaderNames().size());

  // PART headers are not yet implemented
  /*
  auto part0AcceptedEncoding = part0->GetHeader("Content-Disposition");
  ASSERT_EQ("form-data; name=\"A\"", part0AcceptedEncoding)
    << "Content-disposition header must be present for first part (A)";

  auto part1AcceptedEncoding = part1->GetHeader("Content-Disposition");
  ASSERT_EQ("form-data; name=\"B\"", part0AcceptedEncoding)
    << "Content-disposition header must be present for second part (B)";

  auto part0HeaderNameList = part0->GetHeaderNames();
  ASSERT_TRUE(1 == part0HeaderNameList.size())
    << "There must be one header for each part";
  ASSERT_EQ("Content-Disposition", part0HeaderNameList.at(0))
    << "The header named 'Content-disposition' must be there";

  auto part1HeaderNameList = part1->GetHeaderNames();
  ASSERT_TRUE(1 == part1HeaderNameList.size())
    << "There must be one header for each part";
  ASSERT_EQ("Content-Disposition", part1HeaderNameList.at(0))
    << "The header named 'Content-disposition' must be there";
  */
}
