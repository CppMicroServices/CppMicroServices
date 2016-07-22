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

#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usFrameworkEvent.h>

#include <usTestUtils.h>
#include <usTestingMacros.h>
#include <usTestingConfig.h>

#include <iostream>
#include <typeinfo>

using namespace us;

#ifdef ERROR
#undef ERROR	//@todo well this is silly. fix this so it isn't required to compile.
#endif

namespace {

std::string GetMessageFromStdExceptionPtr(const std::exception_ptr ptr)
{
  if (ptr)
  {
    try
    {
      std::rethrow_exception(ptr);
    }
    catch (const std::exception& e)
    {
      return e.what();
    }
  }
  return std::string();
}

}  // end anonymous namespace

int usFrameworkEventTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("FrameworkEventTest");

  // The OSGi spec assigns specific values to event types for future extensibility.
  // Ensure we don't deviate from those assigned values.
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::STARTED == static_cast<FrameworkEvent::Type>(1), "Test assigned event type values");
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::ERROR == static_cast<FrameworkEvent::Type>(2), "Test assigned event type values");
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::INFO == static_cast<FrameworkEvent::Type>(32), "Test assigned event type values");
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::WARNING == static_cast<FrameworkEvent::Type>(16), "Test assigned event type values");
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::STOPPED == static_cast<FrameworkEvent::Type>(64), "Test assigned event type values");
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::WAIT_TIMEDOUT == static_cast<FrameworkEvent::Type>(512), "Test assigned event type values");

  // non-standard to OSGi
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::STOPPING == static_cast<FrameworkEvent::Type>(65), "Test non-standard assigned event type values");
  US_TEST_CONDITION_REQUIRED(FrameworkEvent::Type::STARTING == static_cast<FrameworkEvent::Type>(0), "Test non-standard assigned event type values");

  // @todo mock the framework. We only need a Bundle object to construct a FrameworkEvent object.
  auto const f = FrameworkFactory().NewFramework();

  std::string default_exception_message(GetMessageFromStdExceptionPtr(nullptr));

  FrameworkEvent invalid_event;
  US_TEST_CONDITION_REQUIRED((!invalid_event), "Test for invalid FrameworkEvent construction.");
  US_TEST_CONDITION_REQUIRED(invalid_event.GetType() == FrameworkEvent::Type::STARTING, "invalid event GetType()");
  US_TEST_CONDITION_REQUIRED(invalid_event.GetBundle() == nullptr, "invalid event GetBundle()");
  US_TEST_CONDITION_REQUIRED((GetMessageFromStdExceptionPtr(invalid_event.GetThrowable()) == default_exception_message), "invalid event GetThrowable()");
  std::cout << invalid_event << std::endl;


  FrameworkEvent error_event(FrameworkEvent::Type::ERROR, f, "test framework error event", std::make_exception_ptr(std::runtime_error("test exception")));
  US_TEST_CONDITION_REQUIRED((!error_event) == false, "FrameworkEvent construction - error type");
  US_TEST_CONDITION_REQUIRED(error_event.GetType() == FrameworkEvent::Type::ERROR, "error event GetType()");
  US_TEST_CONDITION_REQUIRED(error_event.GetBundle() == f, "error event GetBundle()");
  US_TEST_CONDITION_REQUIRED(GetMessageFromStdExceptionPtr(error_event.GetThrowable()) == std::string("test exception"), "error event GetThrowable");
  std::cout << error_event << std::endl;

  bool exception_caught = false;
  try
  {
    std::rethrow_exception(error_event.GetThrowable());
  }
  catch (const std::exception& ex)
  {
    exception_caught = true;

    US_TEST_CONDITION_REQUIRED(ex.what() == std::string("test exception"), "Test FrameworkEvent::Type::ERROR exception");
    US_TEST_CONDITION_REQUIRED(typeid(std::runtime_error).name() == typeid(ex).name(), "Test that the correct exception type was thrown");
  }
  US_TEST_CONDITION_REQUIRED(exception_caught, "Test throw/catch a FrameworkEvent exception");

  FrameworkEvent info_event(FrameworkEvent::Type::INFO, f, "test info framework event");
  US_TEST_CONDITION_REQUIRED((!info_event) == false, "FrameworkEvent construction - info type");
  US_TEST_CONDITION_REQUIRED(info_event.GetType() == FrameworkEvent::Type::INFO, "info event GetType()");
  US_TEST_CONDITION_REQUIRED(info_event.GetBundle() == f, "info event GetBundle()");
  US_TEST_CONDITION_REQUIRED(GetMessageFromStdExceptionPtr(info_event.GetThrowable()) == default_exception_message, "info event GetThrowable()");
  std::cout << info_event << std::endl;

  FrameworkEvent warn_event(FrameworkEvent::Type::WARNING, f, "test warning framework event");
  US_TEST_CONDITION_REQUIRED((!warn_event) == false, "FrameworkEvent construction - warning type");
  US_TEST_CONDITION_REQUIRED(warn_event.GetType() == FrameworkEvent::Type::WARNING, "warning event GetType()");
  US_TEST_CONDITION_REQUIRED(warn_event.GetBundle() == f, "warning event GetBundle()");
  US_TEST_CONDITION_REQUIRED(GetMessageFromStdExceptionPtr(warn_event.GetThrowable()) == default_exception_message, "wanring event GetThrowable()");
  std::cout << warn_event << std::endl;

  FrameworkEvent unknown_event(static_cast<FrameworkEvent::Type>(127), f, "test unknown framework event");
  US_TEST_CONDITION_REQUIRED((!unknown_event) == false, "FrameworkEvent construction - unknown type");
  US_TEST_CONDITION_REQUIRED(unknown_event.GetType() == static_cast<FrameworkEvent::Type>(127), "unknown event GetType()");
  US_TEST_CONDITION_REQUIRED(unknown_event.GetBundle() == f, "unknown event GetBundle()");
  US_TEST_CONDITION_REQUIRED(GetMessageFromStdExceptionPtr(unknown_event.GetThrowable()) == default_exception_message, "unknown event GetThrowable()");
  std::cout << unknown_event << std::endl;

  // copy test
  FrameworkEvent dup_error_event(error_event);
  US_TEST_CONDITION_REQUIRED(dup_error_event == error_event, "Test copy ctor");

  // copy assignment test
  dup_error_event = invalid_event;
  US_TEST_CONDITION_REQUIRED(!(dup_error_event == error_event), "Test copy assignment");

  dup_error_event = std::move(FrameworkEvent(FrameworkEvent::Type::STARTED, f, ""));
  US_TEST_CONDITION_REQUIRED(!(dup_error_event == error_event), "Test move");

  US_TEST_END()
}
