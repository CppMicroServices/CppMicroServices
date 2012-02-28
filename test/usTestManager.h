/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#ifndef US_TESTMANAGER_H_INCLUDED
#define US_TESTMANAGER_H_INCLUDED

#include <usConfig.h>

US_BEGIN_NAMESPACE

class TestManager
{
public:

  TestManager() : m_FailedTests(0), m_PassedTests(0) {}
  virtual ~TestManager() {}

  static TestManager& GetInstance();

  /** \brief Must be called at the beginning of a test run. */
  void Initialize();

  int NumberOfFailedTests();
  int NumberOfPassedTests();

  /** \brief Tell manager a subtest failed */
  void TestFailed();

  /** \brief Tell manager a subtest passed */
  void TestPassed();

protected:
  int m_FailedTests;
  int m_PassedTests;
};

US_END_NAMESPACE

#endif // US_TESTMANAGER_H_INCLUDED
