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


#include "usServletContainer.h"

#include <cstdlib>
#include <csignal>

#ifdef US_PLATFORM_WINDOWS
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #define SLEEP(x) Sleep(1000*x)
#else
  #include <unistd.h>
  #define SLEEP(x) sleep(x)
#endif

void signalHandler(int /*num*/)
{
  std::exit(EXIT_SUCCESS);
}

int main(int /*argc*/, const char* /*argv*/[])
{
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGQUIT, signalHandler);
  std::signal(SIGINT, signalHandler);

  us::ServletContainer servletContainer("us");
  servletContainer.Start();

  while(true)
  {
    SLEEP(1);
  }

  return EXIT_SUCCESS;
}
