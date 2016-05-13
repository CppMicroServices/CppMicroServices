/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

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

#ifndef USTESTINGMACROS_H_
#define USTESTINGMACROS_H_

#include <exception>
#include <string>
#include <iostream>
#include <cstdlib>

#include "usTestManager.h"

namespace us {
  /** \brief Indicate a failed test. */
  class TestFailedException : public std::exception {
    public:
      TestFailedException() {}
  };
}

/**
 *
 * \brief Output some text without generating a terminating newline.
 *
 * */
#define US_TEST_OUTPUT_NO_ENDL(x) \
  std::cout x << std::flush;

/** \brief Output some text. */
#define US_TEST_OUTPUT(x) \
  US_TEST_OUTPUT_NO_ENDL(x << "\n")

/** \brief Do some general test preparations. Must be called first in the
     main test function. */
#define US_TEST_BEGIN(testName)                                                               \
  std::string usTestName(#testName);                                                          \
  us::TestManager::GetInstance().Initialize();                                                \
  try {

/** \brief Fail and finish test with message MSG */
#define US_TEST_FAILED_MSG(MSG)                                                               \
  US_TEST_OUTPUT(MSG)                                                                         \
  throw us::TestFailedException();

/** \brief Must be called last in the main test function. */
#define US_TEST_END()                                                                         \
  } catch (const us::TestFailedException&) {                                                  \
    US_TEST_OUTPUT(<< "Further test execution skipped.")                                      \
    us::TestManager::GetInstance().TestFailed();                                              \
  } catch (const std::exception& ex) {                                                        \
    US_TEST_OUTPUT(<< "Exception occured: " << ex.what())                                     \
    us::TestManager::GetInstance().TestFailed();                                              \
  }                                                                                           \
  if (us::TestManager::GetInstance().NumberOfFailedTests() > 0) {                             \
    US_TEST_OUTPUT(<< usTestName << ": [DONE FAILED] , subtests passed: " <<                  \
    us::TestManager::GetInstance().NumberOfPassedTests() << " failed: " <<                    \
    us::TestManager::GetInstance().NumberOfFailedTests() )                                    \
    return EXIT_FAILURE;                                                                      \
  } else {                                                                                    \
    US_TEST_OUTPUT(<< usTestName << ": "                                                      \
                   << us::TestManager::GetInstance().NumberOfPassedTests()                    \
                   << " tests [DONE PASSED]")                                                 \
    return EXIT_SUCCESS;                                                                      \
  }

#define US_TEST_CONDITION(COND,MSG)                                                           \
  US_TEST_OUTPUT_NO_ENDL(<< MSG)                                                              \
  if ( ! (COND) ) {                                                                           \
    us::TestManager::GetInstance().TestFailed();                            \
    US_TEST_OUTPUT(<< " [FAILED]\n" << "In " << __FILE__                                      \
                   << ", line " << __LINE__                                                   \
                   << ":  " #COND " : [FAILED]")                                              \
  } else {                                                                                    \
    US_TEST_OUTPUT(<< " [PASSED]")                                                            \
    us::TestManager::GetInstance().TestPassed();                            \
 }

#define US_TEST_CONDITION_REQUIRED(COND,MSG)                                                  \
  US_TEST_OUTPUT_NO_ENDL(<< MSG)                                                              \
  if ( ! (COND) ) {                                                                           \
    US_TEST_FAILED_MSG(<< " [FAILED]\n" << "  +--> in " << __FILE__                           \
                       << ", line " << __LINE__                                               \
                       << ", expression is false: \"" #COND "\"")                             \
  } else {                                                                                    \
    US_TEST_OUTPUT(<< " [PASSED]")                                                            \
    us::TestManager::GetInstance().TestPassed();                            \
 }

/**
 * \brief Begin block which should be checked for exceptions
 *
 * This macro, together with US_TEST_FOR_EXCEPTION_END, can be used
 * to test whether a code block throws an expected exception. The test FAILS if the
 * exception is NOT thrown.
 */
#define US_TEST_FOR_EXCEPTION_BEGIN(EXCEPTIONCLASS) \
  try {

#define US_TEST_FOR_EXCEPTION_END(EXCEPTIONCLASS)                                             \
    us::TestManager::GetInstance().TestFailed();                            \
    US_TEST_OUTPUT( << "Expected an '" << #EXCEPTIONCLASS << "' exception. [FAILED]")         \
  }                                                                                           \
  catch (EXCEPTIONCLASS) {                                                                    \
    US_TEST_OUTPUT(<< "Caught an expected '" << #EXCEPTIONCLASS                               \
                   << "' exception. [PASSED]")                                                \
    us::TestManager::GetInstance().TestPassed();                            \
  }


/**
 * \brief Simplified version of US_TEST_FOR_EXCEPTION_BEGIN / END for
 * a single statement
 */
#define US_TEST_FOR_EXCEPTION(EXCEPTIONCLASS, STATEMENT)                                      \
  US_TEST_FOR_EXCEPTION_BEGIN(EXCEPTIONCLASS)                                                 \
  STATEMENT ;                                                                                 \
  US_TEST_FOR_EXCEPTION_END(EXCEPTIONCLASS)

#endif // USTESTINGMACROS_H_
