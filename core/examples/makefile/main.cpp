#include <iostream>

#include <usGetBundleContext.h>
#include <usBundleContext.h>
#include <usServiceReference.h>

#include "IDictionaryService.h"

using namespace us;

int main(int /*argc*/, char* /*argv*/[])
{
  ServiceReference<IDictionaryService> dictionaryServiceRef =
    GetBundleContext()->GetServiceReference<IDictionaryService>();
  if (dictionaryServiceRef)
  {
    auto dictionaryService = GetBundleContext()->GetService(dictionaryServiceRef);
    if (dictionaryService)
    {
      std::cout << "Dictionary contains 'Tutorial': " << dictionaryService->CheckWord("Tutorial") << std::endl;
    }
  }
}

#include <usBundleInitialization.h>

US_INITIALIZE_BUNDLE
