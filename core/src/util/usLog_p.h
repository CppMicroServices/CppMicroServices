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

#ifndef USLOG_P_H
#define USLOG_P_H

#include <usCoreConfig.h>
#include <usThreads_p.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>

namespace us {

class LogSink : public MultiThreaded<>, public std::enable_shared_from_this<LogSink> 
{
public:
  explicit LogSink(std::ostream* sink, bool enable = false) : _enable(enable), _sink(sink) 
  {
    if (_sink == nullptr) _enable = false;
  }

  LogSink() = delete;
  LogSink(const LogSink&) = delete;
  LogSink& operator=(const LogSink&) = delete;
  ~LogSink() = default;

  bool Enabled() { return _enable; }

  void Log(const std::string& msg)
  {
    if (!_enable) return;
    auto l = Lock(); US_UNUSED(1);
    *_sink << msg;
  }

private:
  bool _enable;
  std::ostream* const _sink;
};

struct LogMsg {

  LogMsg(LogSink& sink, const char* file, int ln, const char* func)
    : enabled(false), buffer(), _sink(sink)
  { 
	enabled = _sink.Enabled();
	if (enabled)
	{
	  buffer << "In " << func << " at " << file << ":" << ln << " : ";
	}
  }

  LogMsg(const LogMsg& other)
    : enabled(other.enabled), buffer(), _sink(other._sink)
  {}

  ~LogMsg() { if(enabled) _sink.Log(buffer.str()); }

  template<typename T>
  LogMsg& operator<<(const T& t)
  {
    if (enabled) buffer << t;
    return *this;
  }

private:

  bool enabled;
  std::ostringstream buffer;
  LogSink& _sink;
};

}

// Write a log line using a <code>LogSink</code> reference.
#define DIAG_LOG(log_sink) us::LogMsg(log_sink, __FILE__, __LINE__, __FUNCTION__)


#endif // USLOG_P_H
