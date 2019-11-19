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

#include "cppmicroservices/Any.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/shellservice/ShellService.h"
#include "cppmicroservices/shellservice/ShellCommandInterface.h"

#include "linenoise.h"

#include <unordered_map>

// clang-format off
US_GCC_PUSH_DISABLE_WARNING(array-bounds)
// clang-format on

#include "optionparser.h"
US_GCC_POP_WARNING

#include <iostream>

using namespace cppmicroservices;

#define US_SHELL_PROG_NAME "usShell"

enum OptionIndex
{
  UNKNOWN,
  HELP,
  LOAD_BUNDLE
};
const option::Descriptor usage[] = {
  { UNKNOWN,
    0,
    "",
    "",
    option::Arg::None,
    "USAGE: " US_SHELL_PROG_NAME " [options]\n\n"
    "Options:" },
  { HELP,
    0,
    "h",
    "help",
    option::Arg::None,
    "  --help, -h  \tPrint usage and exit." },
  { LOAD_BUNDLE,
    0,
    "l",
    "load",
    option::Arg::Optional,
    "  --load, -l  \tLoad bundle." },
  { UNKNOWN,
    0,
    "",
    "",
    option::Arg::None,
    "\nExamples:\n"
    "  " US_SHELL_PROG_NAME " --load=/home/user/libmybundle.so\n" },
  { 0, 0, nullptr, nullptr, nullptr, nullptr }
};

//static std::shared_ptr<ShellService> g_ShellService;

void shellCompletion(const char* buf, linenoiseCompletions* lc)
{
    if (buf[0] == 'h') {
        linenoiseAddCompletion(lc, "hello");
        linenoiseAddCompletion(lc, "hello there");
    }
//  if (g_ShellService == nullptr || buf == nullptr)
//    return;
//
//  g_ShellService->GetCompletions(buf);
//  std::vector<std::string> completions = g_ShellService->GetCompletions(buf);
//  for (std::vector<std::string>::const_iterator iter = completions.begin(),
//                                                iterEnd = completions.end();
//       iter != iterEnd;
//       ++iter) {
//    linenoiseAddCompletion(lc, iter->c_str());
//  }
}

int main(int argc, char** argv)
{
              argc -= (argc > 0);
              argv += (argc > 0); // skip program name argv[0] if present
              option::Stats stats(usage, argc, argv);
              std::unique_ptr<option::Option[]> options(
                new option::Option[stats.options_max]);
              std::unique_ptr<option::Option[]> buffer(
                new option::Option[stats.buffer_max]);
              option::Parser parse(usage, argc, argv, options.get(), buffer.get());

              if (parse.error())
                return 1;

              if (options[HELP]) {
                option::printUsage(std::cout, usage);
                return 0;
              }

              linenoiseSetCompletionCallback(shellCompletion);

  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Start();
  auto context = framework.GetBundleContext();

  // install ShellService plugin
  const std::string shellservice_path = "/Users/kevinlee/Documents/git/build/lib/Debug/libusShellServiced.dylib";
  context.InstallBundles(shellservice_path);
    
  // start ShellService plugin
  auto installedBundles = context.GetBundles();
  ::cppmicroservices::LDAPFilter shellserviceFilter("(bundle.symbolic_name=usShellService)");
  ::cppmicroservices::Bundle shellservicePlugin;
    
  for (const auto& bundle : installedBundles) {
    if ( shellserviceFilter.Match(bundle) ) {
      shellservicePlugin = bundle;
      break;
    }
  }
    
  shellservicePlugin.Start();
    
  // get service
  auto bc = shellservicePlugin.GetBundleContext();
  std::shared_ptr<ShellService> shellService;
  ServiceReference<ShellService> ref = bc.GetServiceReference<ShellService>();
    
  if (ref) {
    shellService = bc.GetService(ref);
  }
    
  if (!shellService) {
    std::cerr << "Shell service not available" << std::endl;
    return EXIT_FAILURE;
  }
    

  // load user-specified bundles?
  try {
    std::vector<Bundle> bundles;
    for (option::Option* opt = options[LOAD_BUNDLE]; opt; opt = opt->next()) {
      if (opt->arg == nullptr)
        continue;
      std::cout << "Installing " << opt->arg << std::endl;
      auto installedBundles = context.InstallBundles(opt->arg);
      bundles.insert(
        bundles.end(), installedBundles.begin(), installedBundles.end());
    }
    for (auto& bundle : bundles) {
      bundle.Start();
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
    
    
  //g_ShellService = shellService;
    
    
  char* line = nullptr;
  while ((line = linenoise("us> ")) != nullptr) {
    /* Do something with the string. */
    if (line[0] != '\0' && line[0] != '/') {
      linenoiseHistoryAdd(line); /* Add to the history. */
      //linenoiseHistorySave("history.txt"); /* Save the history on disk. */
    }
    
    if (std::string(line) == "quit" | std::string(line) == "exit") {
      break;
    } else if (std::string(line) == "help") {
        auto srShellCommands = context.GetServiceReferences<cppmicroservices::ShellCommandInterface>();
        
        sort(srShellCommands.begin(), srShellCommands.end(), [](const ServiceReference<cppmicroservices::ShellCommandInterface> &lhs, const ServiceReference<cppmicroservices::ShellCommandInterface> &rhs){
            return lhs.GetProperty(Constants::SERVICE_ID).ToString() < rhs.GetProperty(Constants::SERVICE_ID).ToString();
        });
        
        for (auto srShellCommand : srShellCommands) {
            auto svcShellCommand = context.GetService(srShellCommand);
            svcShellCommand->help();
        }
    } else {
      shellService->ExecuteCommand(line);
    }
    free(line);
    std::cout << std::endl;
  }
}
