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

#include "usVariableResolverStreamBuffer_p.h"

#include "usWebConsoleDefaultVariableResolver.h"

#include <cassert>
#include <functional>

namespace us {

VariableResolverStreamBuffer::VariableResolverStreamBuffer(std::ostream* out, WebConsoleVariableResolver* variables)
  : m_State(STATE_NULL)
  , m_Out(out)
  , m_Variables(variables)
{
  if (m_Variables == NULL)
  {
    m_Variables = new WebConsoleDefaultVariableResolver();
  }
  setp(0, 0);
}

VariableResolverStreamBuffer::~VariableResolverStreamBuffer()
{
  delete m_Out;
}

std::streambuf::int_type VariableResolverStreamBuffer::overflow(int_type ch)
{
  if (ch != traits_type::eof())
  {
    assert(std::less_equal<char*>()(pptr(), epptr()));
    if (parse(ch))
    {
      return ch;
    }
  }
  return traits_type::eof();
}

int VariableResolverStreamBuffer::sync()
{
  return 0;
}

bool VariableResolverStreamBuffer::parse(char c)
{
  switch (m_State)
  {
  case STATE_NULL:
    if ( c == '$' )
    {
      m_State = STATE_DOLLAR;
    }
    else if ( c == '\\' )
    {
      m_State = STATE_ESCAPE;
    }
    else
    {
      *m_Out << c;
    }
    break;

  case STATE_DOLLAR:
    if ( c == '{' )
    {
      m_State = STATE_BUFFERING;
    }
    else
    {
      m_State = STATE_NULL;
      *m_Out << '$';
      *m_Out << c;
    }
    break;

  case STATE_BUFFERING:
    if ( c == '}' )
    {
      m_State = STATE_NULL;
      *m_Out << translate();
    }
    else
    {
      m_LineBuffer << c;
    }
    break;

  case STATE_ESCAPE:
    m_State = STATE_NULL;
    if ( c != '$' )
    {
      *m_Out << '\\';
    }
    *m_Out << c;
    break;
  }
  return true;
}

std::string VariableResolverStreamBuffer::translate()
{
  std::string key = m_LineBuffer.str();
  m_LineBuffer.str(std::string());
  m_LineBuffer.clear();

  std::string value = m_Variables->Resolve(key);
  if (value.empty())
  {
    /*
    try
    {
      value = locale.getString( key );
    }
    catch ( MissingResourceException mre )
    */
    {
      // ignore and write the key as the value
      value = key;
    }
  }
  return value;
}

}
