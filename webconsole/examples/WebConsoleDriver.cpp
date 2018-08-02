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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/httpservice/ServletContainer.h"

#include <csignal>
#include <cstdlib>
#include <iostream>

#ifdef US_PLATFORM_WINDOWS
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  define SLEEP(x) Sleep(1000 * x)
#else
#  include <unistd.h>
#  define SLEEP(x) sleep(x)
#endif

void signalHandler(int /*num*/)
{
  std::exit(EXIT_SUCCESS);
}

int main(int argc, const char* argv[])
{
  std::signal(SIGTERM, signalHandler);
  std::signal(SIGINT, signalHandler);
#ifndef WIN32
  std::signal(SIGQUIT, signalHandler);
#endif

  if (argc < 2) {
    std::cout << "No command line arguments given.\n"
                 "Provide a space separated list of file paths pointing to "
                 "bundles to load.\n"
                 "The web console driver needs at least the httpservice and "
                 "webconsole bundle.\n";
    return 0;
  }

  cppmicroservices::FrameworkFactory fwFactory;
  auto fw = fwFactory.NewFramework();
  fw.Start();

  auto ctx = fw.GetBundleContext();

  for (int i = 1; i < argc; ++i) {
    ctx.InstallBundles(argv[i]);
  }

  for (auto& b : ctx.GetBundles()) {
    b.Start();
  }

  cppmicroservices::ServletContainer servletContainer(ctx, "us");
  servletContainer.Start();

  while (true) {
    SLEEP(1);
  }

  return EXIT_SUCCESS;
}
