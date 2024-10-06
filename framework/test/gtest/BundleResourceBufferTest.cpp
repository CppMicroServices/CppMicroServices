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

#include <chrono>
#include <fstream>
#include <mutex>
#include <thread>
#include <type_traits>

#include "TestUtilBundleListener.h"
#include "TestUtils.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/util/FileSystem.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Mocks.h"

using namespace cppmicroservices::detail;

// A custom deleter for the unique_ptr used in BundleResourceBuffer
void customDeleter(void* ptr)
{
    std::free(ptr);
}

namespace cppmicroservices
{
    namespace detail
    {
        class BundleResourceBufferTestSuite
        {
          public:
            std::unique_ptr<void, void(*)(void*)> CreateTestData(const std::string& data)
            {
                void* rawData = std::malloc(data.size());
                std::memcpy(rawData, data.data(), data.size());
                return std::unique_ptr<void, void(*)(void*)>(rawData, customDeleter);
            }

            void UnderflowAndUflow()
            {
                std::string testData = "Hello, World!";
                auto data = CreateTestData(testData);
                BundleResourceBuffer buffer(std::move(data), testData.size(), std::ios_base::in);

                EXPECT_EQ(buffer.underflow(), 'H');
                EXPECT_EQ(buffer.uflow(), 'H');
                EXPECT_EQ(buffer.uflow(), 'e');
            }

            void Pbackfail()
            {
                std::string testData = "Hello";
                auto data = CreateTestData(testData);
                BundleResourceBuffer buffer(std::move(data), testData.size(), std::ios_base::in);

                buffer.uflow(); // Read 'H'
                EXPECT_EQ(buffer.pbackfail('H'), 'H');
                EXPECT_EQ(buffer.uflow(), 'H'); // Should read 'H' again
            }

            void Showmanyc()
            {
                std::string testData = "Hello\nWorld";
                auto data = CreateTestData(testData);
                BundleResourceBuffer buffer(std::move(data), testData.size(), std::ios_base::in);


                EXPECT_EQ(buffer.showmanyc(), testData.size());
                buffer.uflow(); // Read 'H'
                EXPECT_EQ(buffer.showmanyc(), testData.size() - 1);
            }

            void SeekoffAndSeekpos()
            {
                std::string testData = "Hello, World!";
                auto data = CreateTestData(testData);
                BundleResourceBuffer buffer(std::move(data), testData.size(), std::ios_base::in);

                EXPECT_EQ(buffer.uflow(), 'H');
                buffer.seekoff(7, std::ios_base::cur);
                EXPECT_EQ(buffer.uflow(), 111);
                buffer.seekpos(0);
                EXPECT_EQ(buffer.uflow(), 'H');
            }

            void NewlineConversion()
            {
                std::string testData = "Line1\r\nLine2\r\n";
                auto data = CreateTestData(testData);
                BundleResourceBuffer buffer(std::move(data), testData.size(), std::ios_base::in);

                EXPECT_EQ(buffer.uflow(), 'L');
                buffer.seekpos(5);

                EXPECT_EQ(buffer.uflow(), 'L'); // Should skip the '\r'
            }

            void RemoveLastNewline()
            {
                std::string testData = "Line1\nLine2\n";

                auto data = CreateTestData(testData);
                BundleResourceBuffer buffer(std::move(data), testData.size(), std::ios_base::in);

                buffer.seekpos(testData.size() - 1);
                EXPECT_EQ(buffer.uflow(), std::char_traits<char>::eof()); // Last newline should be removed
            }
        };
        
        class BundleResourceBufferTest : public ::testing::Test {};

        TEST_F(BundleResourceBufferTest, UnderflowAndUflow)
        {
            BundleResourceBufferTestSuite suite;
            suite.UnderflowAndUflow();
        }

        TEST_F(BundleResourceBufferTest, Pbackfail)
        {
            BundleResourceBufferTestSuite suite;
            suite.Pbackfail();
        }


        TEST_F(BundleResourceBufferTest, Showmanyc)
        {
            BundleResourceBufferTestSuite suite;
            suite.Showmanyc();
        }

        TEST_F(BundleResourceBufferTest, SeekoffAndSeekpos)
        {
            BundleResourceBufferTestSuite suite;
            suite.SeekoffAndSeekpos();
        }

#ifdef DATA_NEEDS_NEWLINE_CONVERSION
        TEST_F(BundleResourceBufferTest, NewlineConversion)
        {
            BundleResourceBufferTestSuite suite;
            suite.NewlineConversion();
        }
#endif

#ifdef REMOVE_LAST_NEWLINE_IN_TEXT_MODE
        TEST_F(BundleResourceBufferTest, RemoveLastNewline)
        {
            BundleResourceBufferTestSuite suite;
            suite.RemoveLastNewline();
        }
#endif
    } // namespace detail
} // namespace cppmicroservices
