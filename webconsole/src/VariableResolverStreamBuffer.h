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

#ifndef CPPMICROSERVICES_VARIABLERESOLVERSTREAMBUFFER_H
#define CPPMICROSERVICES_VARIABLERESOLVERSTREAMBUFFER_H

#include "cppmicroservices/GlobalConfig.h"

#include <streambuf>
#include <sstream>

namespace cppmicroservices {

struct WebConsoleVariableResolver;

class VariableResolverStreamBuffer : public std::streambuf
{
public:
  explicit VariableResolverStreamBuffer(std::ostream* out, WebConsoleVariableResolver* variables);
  ~VariableResolverStreamBuffer();

private:

  int_type overflow(int_type ch);

  int sync();

  /**
       * Write a single character following the state machine:
       * <table>
       * <tr><th>State</th><th>Character</th><th>Task</th><th>Next State</th></tr>
       * <tr><td>NULL</td><td>$</td><td>&nbsp;</td><td>DOLLAR</td></tr>
       * <tr><td>NULL</td><td>\</td><td>&nbsp;</td><td>ESCAPE</td></tr>
       * <tr><td>NULL</td><td>any</td><td>write c</td><td>NULL</td></tr>
       * <tr><td>DOLLAR</td><td>{</td><td>&nbsp;</td><td>BUFFERING</td></tr>
       * <tr><td>DOLLAR</td><td>any</td><td>write $ and c</td><td>NULL</td></tr>
       * <tr><td>BUFFERING</td><td>}</td><td>translate and write translation</td><td>NULL</td></tr>
       * <tr><td>BUFFERING</td><td>any</td><td>buffer c</td><td>BUFFERING</td></tr>
       * <tr><td>ESACPE</td><td>$</td><td>write $</td><td>NULL</td></tr>
       * <tr><td>ESCAPE</td><td>any</td><td>write \ and c</td><td>NULL</td></tr>
       * </table>
       *
       * @exception IOException If an I/O error occurs
       */
  bool parse(char c);

  std::string translate();

  VariableResolverStreamBuffer(const VariableResolverStreamBuffer&);
  VariableResolverStreamBuffer& operator=(const VariableResolverStreamBuffer&);

private:

  enum State {
  /**
       * normal processing state, $ signs are recognized here
       * proceeds to {@link #STATE_DOLLAR} if a $ sign is encountered
       * proceeds to {@link #STATE_ESCAPE} if a \ sign is encountered
       * otherwise just writes the character
       */
    STATE_NULL,

      /**
       * State after a $ sign has been recognized
       * proceeds to {@value #STATE_BUFFERING} if a { sign is encountered
       * otherwise proceeds to {@link #STATE_NULL} and writes the $ sign and
       * the current character
       */
     STATE_DOLLAR,

      /**
       * buffers characters until a } is encountered
       * proceeds to {@link #STATE_NULL} if a } sign is encountered and
       * translates and writes buffered text before returning
       * otherwise collects characters to gather the translation key
       */
      STATE_BUFFERING,

      /**
       * escaping the next character, if the character is a $ sign, the
       * $ sign is writeted. otherwise the \ and the next character is
       * written
       * proceeds to {@link #STATE_NULL}
       */
      STATE_ESCAPE = 3
  };

      /**
       * The current state, starts with {@link #STATE_NULL}
       */
  State m_State;

  std::ostream* m_Out;
  WebConsoleVariableResolver* m_Variables;
  char m_Buffer;
  std::stringstream m_LineBuffer;
};

}

#endif // CPPMICROSERVICES_VARIABLERESOLVERSTREAMBUFFER_H
