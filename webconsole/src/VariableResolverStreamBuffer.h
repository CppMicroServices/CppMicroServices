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

#ifndef CPPMICROSERVICES_VARIABLERESOLVERSTREAMBUFFER_H
#define CPPMICROSERVICES_VARIABLERESOLVERSTREAMBUFFER_H

#include "cppmicroservices/GlobalConfig.h"
#include "cppmicroservices/webconsole/WebConsoleVariableResolver.h"

#include <memory>
#include <sstream>
#include <streambuf>

namespace cppmicroservices {

class VariableResolverStreamBuffer : public std::streambuf
{
public:
  explicit VariableResolverStreamBuffer(
    std::unique_ptr<std::ostream> out,
    std::shared_ptr<WebConsoleVariableResolver>  resolver);

  ~VariableResolverStreamBuffer();

  VariableResolverStreamBuffer(const VariableResolverStreamBuffer&) = delete;
  VariableResolverStreamBuffer& operator=(const VariableResolverStreamBuffer&) =
    delete;

private:
  int_type overflow(int_type ch);

  int sync();

  /**
   * Write a single character following the state machine:
   *
   * State                       | Character | Task                                   | Next State
   * ------------------------------------------------------------------------------------------------------------
   * NIL                         | {         |                                        | OBRACE
   * NIL                         | any       | write c                                | NIL
   * OBRACE                      | {         | buffer {{                              | MUSTACHE
   * OBRACE                      | any       | write { and c                          | NIL
   * MUSTACHE                    | # or ^    | buffer c                               | MUSTACHE_BLOCK_BEGIN
   * MUSTACHE                    | any       | buffer c                               | MUSTACHE_VAR
   * MUSTACHE_VAR                | }         | buffer c                               | MUSTACHE_VAR_CBRACE
   * MUSTACHE_VAR                | any       | buffer c                               | MUSTACHE_VAR
   * MUSTACHE_VAR_CBRACE         | }         | buffer c, render buffer                | NIL
   * MUSTACHE_VAR_CBRACE         | any       | buffer c                               | MUSTACHE_VAR
   * MUSTACHE_BLOCK_BEGIN        | }         | buffer c                               | MUSTACHE_BLOCK_BEGIN_CBRACE
   * MUSTACHE_BLOCK_BEGIN        | any       | buffer c, btag c                       | MUSTACHE_BLOCK_BEGIN
   * MUSTACHE_BLOCK_BEGIN_CBRACE | }         | buffer c                               | MUSTACHE_BLOCK
   * MUSTACHE_BLOCK_BEGIN_CBRACE | any       | buffer c, btag } and c                 | MUSTACHE_BLOCK_BEGIN
   * MUSTACHE_BLOCK              | {         | buffer c                               | MUSTACHE_BLOCK_OBRACE
   * MUSTACHE_BLOCK              | any       | buffer c                               | MUSTACHE_BLOCK
   * MUSTACHE_BLOCK_OBRACE       | {         | buffer c                               | MUSTACHE_BLOCK_MUSTACHE
   * MUSTACHE_BLOCK_OBRACE       | any       | buffer c                               | MUSTACHE_BLOCK
   * MUSTACHE_BLOCK_MUSTACHE     | /         | buffer c                               | MUSTACHE_BLOCK_END
   * MUSTACHE_BLOCK_MUSTACHE     | any       | buffer c                               | MUSTACHE BLOCK
   * MUSTACHE_BLOCK_END          | }         | buffer c                               | MUSTACHE_BLOCK_END_CBRACE
   * MUSTACHE_BLOCK_END          | any       | buffer c, etag c                       | MUSTACHE_BLOCK_END
   * MUSTACHE_BLOCK_END_CBRACE   | }         | buffer c, if tags match render buffer  | if match NIL else MUSTACHE_BLOCK
   * MUSTACHE_BLOCK_END_CBRACE   | any       | buffer c, etag } and c                 | MUSTACHE_BLOCK END
   *
   * @exception IOException If an I/O error occurs
   */
  bool Parse(char c);

  void Translate();

  enum class State
  {

    NIL,
    OBRACE,
    MUSTACHE,
    MUSTACHE_VAR,
    MUSTACHE_VAR_CBRACE,
    MUSTACHE_BLOCK_BEGIN,
    MUSTACHE_BLOCK_BEGIN_CBRACE,
    MUSTACHE_BLOCK,
    MUSTACHE_BLOCK_OBRACE,
    MUSTACHE_BLOCK_MUSTACHE,
    MUSTACHE_BLOCK_END,
    MUSTACHE_BLOCK_END_CBRACE

  };

  /**
   * The current state, starts with State::NIL
   */
  State m_State;

  std::unique_ptr<std::ostream> m_Out;

  std::shared_ptr<WebConsoleVariableResolver> m_Variables;
  std::stringstream m_Buffer;
  std::stringstream m_BeginTag;
  std::stringstream m_EndTag;
};
}

#endif // CPPMICROSERVICES_VARIABLERESOLVERSTREAMBUFFER_H
