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

#include <usLog_p.h>

#include "usTestingMacros.h"

US_USE_NAMESPACE

static int lastMsgType = -1;
static std::string lastMsg;

void handleMessages(MsgType type, const char* msg)
{
  lastMsgType = type;
  lastMsg.assign(msg);
}

void resetLastMsg()
{
  lastMsgType = -1;
  lastMsg.clear();
}

void logMsg(MsgType msgType, int logLevel)
{
  resetLastMsg();
  std::string logMsg;
  switch (msgType)
  {
  case DebugMsg:
  {
    logMsg = "Debug msg";
    US_DEBUG << logMsg;
#if !defined(US_ENABLE_DEBUG_OUTPUT)
    if (logLevel == DebugMsg) logLevel = InfoMsg;
#endif
    break;
  }
  case InfoMsg:
  {
    logMsg = "Info msg";
    US_INFO << logMsg;
    break;
  }
  case WarningMsg:
  {
    logMsg = "Warning msg";
    US_WARN << logMsg;
    break;
  }
  case ErrorMsg:
  {
    // Skip error messages
    logLevel = 100;
    break;
  }
  }

  if (msgType >= logLevel)
  {
    US_TEST_CONDITION(lastMsgType == msgType && lastMsg.find(logMsg) != std::string::npos, "Testing for logged message")
  }
  else
  {
    US_TEST_CONDITION(lastMsgType == -1 && lastMsg.empty(), "Testing for skipped log message")
  }
}

void testLogMessages()
{
  // Use the default message handler
  installMsgHandler(0);
  {
    US_DEBUG << "Msg";
    US_DEBUG(false) << "Msg";
    US_INFO << "Msg";
    US_INFO(false) << "Msg";
    US_WARN << "Msg";
    US_WARN(false) << "Msg";
  }
  US_TEST_CONDITION(lastMsg.empty(), "Testing default message handler");

  resetLastMsg();

  installMsgHandler(handleMessages);
  {
    US_DEBUG << "Msg";
  }
#if !defined(US_ENABLE_DEBUG_OUTPUT)
  US_TEST_CONDITION(lastMsgType == -1 && lastMsg.empty(), "Testing suppressed debug message")
#else
  US_TEST_CONDITION(lastMsgType == 0 && lastMsg.find("Msg") != std::string::npos, "Testing debug message")
#endif
  resetLastMsg();

  {
    US_DEBUG(false) << "No msg";
  }
  US_TEST_CONDITION(lastMsgType == -1 && lastMsg.empty(), "Testing disabled debug message")
  resetLastMsg();

  {
    US_INFO << "Info msg";
  }
  US_TEST_CONDITION(lastMsgType == 1 && lastMsg.find("Info msg") != std::string::npos, "Testing informational message")
  resetLastMsg();

  {
    US_WARN << "Warn msg";
  }
  US_TEST_CONDITION(lastMsgType == 2 && lastMsg.find("Warn msg") != std::string::npos, "Testing warning message")
  resetLastMsg();

  // We cannot test US_ERROR since it will call abort().

  installMsgHandler(0);

  {
    US_INFO << "Info msg";
  }
  US_TEST_CONDITION(lastMsgType == -1 && lastMsg.empty(), "Testing message handler reset")
  resetLastMsg();
}

void testLogLevels()
{
  installMsgHandler(handleMessages);

  MsgType logLevel = ModuleSettings::GetLogLevel();
  US_TEST_CONDITION_REQUIRED(logLevel == DebugMsg, "Default log level")

  logMsg(DebugMsg, logLevel);
  logMsg(InfoMsg, logLevel);
  logMsg(WarningMsg, logLevel);
  logMsg(ErrorMsg, logLevel);

  for (int level = ErrorMsg; level >= 0; --level)
  {
    ModuleSettings::SetLogLevel(static_cast<MsgType>(level));
    logMsg(DebugMsg, level);
    logMsg(InfoMsg, level);
    logMsg(WarningMsg, level);
    logMsg(ErrorMsg, level);
  }

  installMsgHandler(0);
  resetLastMsg();
}

int usLogTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("DebugOutputTest");

  testLogMessages();
  testLogLevels();

  US_TEST_END()
}
