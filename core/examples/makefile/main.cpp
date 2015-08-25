#include <usGetBundleContext.h>
#include <usBundleContext.h>
#include <usServiceReference.h>

#include "IDictionaryService.h"

US_USE_NAMESPACE

int main(int /*argc*/, char* /*argv*/[])
{
  ServiceReference<IDictionaryService> dictionaryServiceRef =
    GetBundleContext()->GetServiceReference<IDictionaryService>();
  if (dictionaryServiceRef)
  {
    IDictionaryService* dictionaryService = GetBundleContext()->GetService(dictionaryServiceRef);
    if (dictionaryService)
    {
      std::cout << "Dictionary contains 'Tutorial': " << dictionaryService->CheckWord("Tutorial") << std::endl;
    }
  }
}

#include <usBundleInitialization.h>

US_INITIALIZE_BUNDLE
