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

#include "usModuleInitialization.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usAny.h"

#include "usShellService.h"

#include "linenoise.h"

#include <iostream>

US_USE_NAMESPACE

static ShellService* g_ShellService = NULL;

void shellCompletion(const char* buf, linenoiseCompletions* lc)
{
  if (g_ShellService == NULL || buf == NULL) return;

  g_ShellService->GetCompletions(buf);
  std::vector<std::string> completions = g_ShellService->GetCompletions(buf);
  for (std::vector<std::string>::const_iterator iter = completions.begin(),
       iterEnd = completions.end(); iter != iterEnd; ++iter)
  {
    linenoiseAddCompletion(lc, iter->c_str());
  }
}

int main(int /*argc*/, char** /*argv*/)
{
  linenoiseSetCompletionCallback(shellCompletion);

  ModuleContext* context = GetModuleContext();
  ShellService* shellService = NULL;
  ServiceReference<ShellService> ref = context->GetServiceReference<ShellService>();
  if (ref)
  {
    shellService = context->GetService(ref);
  }

  if (shellService == NULL)
  {
    std::cerr << "Shell service not available" << std::endl;
    return EXIT_FAILURE;
  }

  g_ShellService = shellService;

  char* line = NULL;
  while((line = linenoise("us> ")) != NULL)
  {
    /* Do something with the string. */
    if (line[0] != '\0' && line[0] != '/')
    {
      linenoiseHistoryAdd(line); /* Add to the history. */
      //linenoiseHistorySave("history.txt"); /* Save the history on disk. */
    }
    shellService->ExecuteCommand(line);
    free(line);
  }
}

US_INITIALIZE_MODULE
