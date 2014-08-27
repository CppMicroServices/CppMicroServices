#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include "IDictionaryService.h"

US_USE_NAMESPACE

int main(int /*argc*/, char* /*argv*/[])
{
  ServiceReference<IDictionaryService> dictionaryServiceRef =
    GetModuleContext()->GetServiceReference<IDictionaryService>();
  if (dictionaryServiceRef)
  {
    IDictionaryService* dictionaryService = GetModuleContext()->GetService(dictionaryServiceRef);
    if (dictionaryService)
    {
      std::cout << "Dictionary contains 'Tutorial': " << dictionaryService->CheckWord("Tutorial") << std::endl;
    }
  }
}

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE
