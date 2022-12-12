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

#include "cppmicroservices/detail/Log.h"

#include "gtest/gtest.h"

#include <fstream>
#include <regex>
#include <thread>

using namespace cppmicroservices;

TEST(LogTest, testDefaultLogMessages)
{
    std::stringstream temp_buf;
    auto clog_buf = std::clog.rdbuf();
    std::clog.rdbuf(temp_buf.rdbuf());
    detail::LogSink sink(&std::clog, true);
    DIAG_LOG(sink) << "Testing " << 1 << 2 << 3 << ", Testing " << std::scientific << 1.0 << static_cast<void*>(nullptr)
                   << 2.0 << 3.0 << "\n";
    sink.Log(std::string("blaaaaaaaaaaaaaaaaaah\n"));

    std::clog.rdbuf(clog_buf);

    // Test default log sink
    ASSERT_NE(std::string::npos, temp_buf.str().find(std::string("blaaaaaaaaaaaaaaaaaah\n")));

    // Test default log sink macro
    ASSERT_NE(std::string::npos, temp_buf.str().find(std::string(__FUNCTION__)));
    ASSERT_NE(std::string::npos, temp_buf.str().find(std::string(__FILE__)));
}

TEST(LogTest, testLogDisabled)
{
    // A null (i.e. disabled logger) shouldn't throw when used.
    detail::LogSink sink_null(nullptr);

    EXPECT_NO_THROW(sink_null.Log(std::string("Don't log me"))) << "Using a nullptr log sink threw an exception.";
    EXPECT_NO_THROW(DIAG_LOG(sink_null));

    std::ostringstream empty_stream;
    detail::LogSink sink_disabled(&empty_stream);
    DIAG_LOG(sink_disabled) << "Now you see me...";
    sink_disabled.Log(std::string("Now you don't"));

    // Test disabled log sink
    ASSERT_TRUE(empty_stream.str().empty());
}

TEST(LogTest, testLogRedirection)
{
    char const* test_filename = "foo.txt";
    std::ofstream filestream(test_filename, std::ofstream::trunc);
    std::ostringstream stringstream;
    std::stringstream test_log_output;
    test_log_output << "Testing..." << 1 << " " << 2 << " " << 12 << "\n";

    detail::LogSink sink_stringstream(&stringstream, true);
    DIAG_LOG(sink_stringstream) << test_log_output.str();

    // Test redirected stringstream log sink
    ASSERT_FALSE(stringstream.str().empty());
    ASSERT_NE(std::string::npos, stringstream.str().find(test_log_output.str()));

    detail::LogSink sink_file(&filestream, true);
    DIAG_LOG(sink_file) << test_log_output.str();
    filestream.flush();

    std::ifstream test_file(test_filename);
    std::stringstream test_output_stream;
    test_file >> test_output_stream.rdbuf();

    // Test redirected filestream log sink
    ASSERT_FALSE(test_output_stream.str().empty());
    ASSERT_NE(std::string::npos, test_output_stream.str().find(test_log_output.str()));

    test_file.close();
    if (std::remove(test_filename) != 0)
    {
        detail::LogSink sink_err(&std::cerr, true);
        DIAG_LOG(sink_err) << "Failed to remove " << test_filename
                           << "\nAbove message is as expected with 'DIAG_LOG(sink_err)'\n";
    }

    // A null (i.e. disabled logger) shouldn't throw when used.
    detail::LogSink sink_null(nullptr, true);
    EXPECT_NO_THROW(DIAG_LOG(sink_null)) << "Using a nullptr log sink threw an exception";

    std::ostringstream local_cerr_buffer;
    auto cerr_buffer = std::cerr.rdbuf();
    std::cerr.rdbuf(local_cerr_buffer.rdbuf());

    detail::LogSink sink_redirected_cerr(&std::cerr, true);
    DIAG_LOG(sink_redirected_cerr) << test_log_output.str();
    std::cerr.rdbuf(cerr_buffer);

    // Test redirected std::cerr log sink
    ASSERT_FALSE(local_cerr_buffer.str().empty());
    ASSERT_NE(std::string::npos, local_cerr_buffer.str().find(test_log_output.str()));
}

#ifdef US_ENABLE_THREADING_SUPPORT
// hammer the logger from multiple threads. A failure in
// thread safety will most likely manifest as either a crash
// or the output validation will see splicing of log lines.
TEST(LogTest, testLogMultiThreaded)
{
    std::stringstream stringstream;
    detail::LogSink sink(&stringstream, true);

    std::size_t num_threads(100);
    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < num_threads; ++i)
    {
        threads.push_back(std::thread(
            [&sink, &num_threads]()
            {
                for (std::size_t i = 0; i < num_threads; ++i)
                {
                    DIAG_LOG(sink) << "MACRO: START foo\n";
                    sink.Log(std::string("foo bar boo baz\n"));
                    DIAG_LOG(sink) << "MACRO: END foo\n";
                }
            }));
    }

    for (auto& t : threads)
        t.join();

    // instead of testing for all three log lines in one regular expressions, test for each line
    // as logging a single line is expected to be thread safe. It is NOT guaranteed that all three
    // log lines will appear in the correct order.
    std::ptrdiff_t expected_num_matches(num_threads * num_threads);
#    if defined(US_HAVE_REGEX)
    std::string func_name(__FUNCTION__);
    std::string file_name = std::regex_replace(std::string(__FILE__), std::regex("\\\\"), std::string("\\\\"));
    std::string logpreamble("In (" + func_name + "::<lambda(\\w+)>::)*operator(\\s)?\\(\\) at " + file_name
                            + ":(\\d+) :");
    std::regex reg_expr_start(logpreamble + std::string(" MACRO: START foo\n"));
    std::regex reg_expr_middle("foo bar boo baz\n");
    std::regex reg_expr_end(logpreamble + std::string(" MACRO: END foo\n"));

    const std::string stream(stringstream.str());
    auto regex_iter_end = std::sregex_iterator();

    auto regex_iter_begin = std::sregex_iterator(stream.begin(), stream.end(), reg_expr_start);
    std::ptrdiff_t num_found = std::distance(regex_iter_begin, regex_iter_end);

    // Test for expected number of matches
    ASSERT_EQ(num_found, expected_num_matches);

    regex_iter_begin = std::sregex_iterator(stream.begin(), stream.end(), reg_expr_middle);
    num_found = std::distance(regex_iter_begin, regex_iter_end);
    // Test for expected number of matches
    ASSERT_EQ(num_found, expected_num_matches);

    regex_iter_begin = std::sregex_iterator(stream.begin(), stream.end(), reg_expr_end);
    num_found = std::distance(regex_iter_begin, regex_iter_end);
    // Test for expected number of matches
    ASSERT_EQ(num_found, expected_num_matches);

#    else
    // support compilers w/o c++11 regex support...
    // the regex approach is more strict in checking there is no
    // log splicing however this suboptimal approach will do for now.
    std::ptrdiff_t total_expected_matches = expected_num_matches * 3;
    std::vector<std::string> log_lines;
    std::string line;
    while (std::getline(stringstream, line, '\n'))
    {
        log_lines.push_back(line);
    }
    // Test for expected number of matches
    ASSERT_EQ(static_cast<std::ptrdiff_t>(log_lines.size()), total_expected_matches);

    std::ptrdiff_t count = std::count_if(log_lines.begin(),
                                         log_lines.end(),
                                         [](std::string const& s)
                                         { return (std::string::npos != s.find(std::string("MACRO: START foo"))); });
    // Test for expected number of matches
    ASSERT_EQ(count, expected_num_matches);

    count = std::count_if(log_lines.begin(),
                          log_lines.end(),
                          [](std::string const& s)
                          { return (std::string::npos != s.find(std::string("MACRO: END foo"))); });
    // Test for expected number of matches
    ASSERT_EQ(count, expected_num_matches);

    count = std::count_if(log_lines.begin(),
                          log_lines.end(),
                          [](std::string const& s) { return (s == std::string("foo bar boo baz")); });
    // Test for expected number of matches
    ASSERT_EQ(count, expected_num_matches);
#    endif
}
#endif
