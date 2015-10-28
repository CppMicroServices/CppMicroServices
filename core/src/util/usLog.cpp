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

#include <usLog.h>

namespace us {

    Logger::Logger(void) : logLevel(DebugMsg) {}
    Logger::~Logger() {}
    Logger& Logger::instance()
    {
        /*
        IMPORTANT: This is only thread safe if compiling with a C++11 compiler
        which implements C++11 standard section 6.7.4:
        "If control enters the declaration concurrently while the variable is being initialized,
        the concurrent execution shall wait for completion of the initialization."

        However, even if a C++11 supported compiler isn't used, thread safe initialization
        of function local statics isn't a concern at the moment becasue this Logger
        is used internally by the Framework and not instantiated in a concurrent fashion.
        */
        static Logger inst;
        return inst;
    }

    /**
    * Set the logging level for log messages from CppMicroServices bundles.
    *
    * Higher logging levels will discard messages with lower priority.
    * E.g. a logging level of WarningMsg will discard all messages of
    * type DebugMsg and InfoMsg.
    *
    * @param level The new logging level.
    */
    void Logger::SetLogLevel(const MsgType level)
    {
        Lock l(this);
        logLevel = level;
    }

    /**
    * Get the current logging level.
    *
    * @return The currently used logging level.
    */
    MsgType Logger::GetLogLevel()
    {
        Lock l(this);
        return logLevel;
    }

}