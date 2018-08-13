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

#include "VariableResolverStreamBuffer.h"

#include "cppmicroservices/webconsole/WebConsoleDefaultVariableResolver.h"

#include <cassert>
#include <functional>
#include <utility>

namespace cppmicroservices {

VariableResolverStreamBuffer::VariableResolverStreamBuffer(
  std::unique_ptr<std::ostream> out,
  std::shared_ptr<WebConsoleVariableResolver>  variables)
  : m_State(State::NIL)
  , m_Out(std::move(out))
  , m_Variables(std::move(variables))
{
  if (!m_Variables) {
    m_Variables = std::make_shared<WebConsoleDefaultVariableResolver>();
  }
  setp(nullptr, nullptr);
}

VariableResolverStreamBuffer::~VariableResolverStreamBuffer() = default;

std::streambuf::int_type VariableResolverStreamBuffer::overflow(int_type ch)
{
  if (ch != traits_type::eof()) {
    assert(std::less_equal<char*>()(pptr(), epptr()));
    if (Parse(ch)) {
      return ch;
    }
  }
  return traits_type::eof();
}

int VariableResolverStreamBuffer::sync()
{
  return 0;
}

bool VariableResolverStreamBuffer::Parse(char c)
{
  switch (m_State) {
    case State::NIL:
      if (c == '{') {
        m_State = State::OBRACE;
      } else {
        *m_Out << c;
      }
      break;

    case State::OBRACE:
      if (c == '{') {
        m_State = State::MUSTACHE;
        m_Buffer << "{{";
      } else {
        m_State = State::NIL;
        *m_Out << '{';
        *m_Out << c;
      }
      break;

    case State::MUSTACHE:
      m_Buffer << c;
      if (c == '#' || c == '^') {
        m_State = State::MUSTACHE_BLOCK_BEGIN;
      } else {
        m_State = State::MUSTACHE_VAR;
      }
      break;

    case State::MUSTACHE_VAR:
      m_Buffer << c;
      if (c == '}') {
        m_State = State::MUSTACHE_VAR_CBRACE;
      }
      break;

    case State::MUSTACHE_VAR_CBRACE:
      m_Buffer << c;
      if (c == '}') {
        Translate();
        m_State = State::NIL;
      } else {
        m_State = State::MUSTACHE_VAR;
      }
      break;

    case State::MUSTACHE_BLOCK_BEGIN:
      m_Buffer << c;
      if (c == '}') {
        m_State = State::MUSTACHE_BLOCK_BEGIN_CBRACE;
      } else {
        m_BeginTag << c;
      }
      break;

    case State::MUSTACHE_BLOCK_BEGIN_CBRACE:
      m_Buffer << c;
      if (c == '}') {
        m_State = State::MUSTACHE_BLOCK;
      } else {
        m_BeginTag << '}';
        m_BeginTag << c;
        m_State = State::MUSTACHE_BLOCK_BEGIN;
      }
      break;

    case State::MUSTACHE_BLOCK:
      m_Buffer << c;
      if (c == '{') {
        m_State = State::MUSTACHE_BLOCK_OBRACE;
      }
      break;

    case State::MUSTACHE_BLOCK_OBRACE:
      m_Buffer << c;
      if (c == '{') {
        m_State = State::MUSTACHE_BLOCK_MUSTACHE;
      } else {
        m_State = State::MUSTACHE_BLOCK;
      }
      break;

    case State::MUSTACHE_BLOCK_MUSTACHE:
      m_Buffer << c;
      if (c == '/') {
        m_State = State::MUSTACHE_BLOCK_END;
      } else {
        m_State = State::MUSTACHE_BLOCK;
      }
      break;

    case State::MUSTACHE_BLOCK_END:
      m_Buffer << c;
      if (c == '}') {
        m_State = State::MUSTACHE_BLOCK_END_CBRACE;
      } else {
        m_EndTag << c;
      }
      break;

    case State::MUSTACHE_BLOCK_END_CBRACE:
      m_Buffer << c;
      if (c == '}') {
        if (m_BeginTag.str() == m_EndTag.str()) {
          Translate();
          m_BeginTag.str(std::string());
          m_State = State::NIL;
        } else {
          m_State = State::MUSTACHE_BLOCK;
        }
        m_EndTag.str(std::string());
      } else {
        m_EndTag << '}';
        m_EndTag << c;
        m_State = State::MUSTACHE_BLOCK_END;
      }
      break;
  }
  return true;
}

void VariableResolverStreamBuffer::Translate()
{
  std::string buf = m_Buffer.str();
  m_Buffer.str(std::string());

  *m_Out << m_Variables->Resolve(buf);
}
}
