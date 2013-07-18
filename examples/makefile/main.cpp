#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceReference.h>

#include US_BASECLASS_HEADER

#include "IDictionaryService.h"

US_USE_NAMESPACE

int main(int argc, char* argv[])
{
  ServiceReference dictionaryServiceRef = GetModuleContext()->GetServiceReference<IDictionaryService>();
  if (dictionaryServiceRef)
  {
    IDictionaryService* dictionaryService = GetModuleContext()->GetService<IDictionaryService>(dictionaryServiceRef);
    if (dictionaryService)
    {
      std::cout << "Dictionary contains 'Tutorial': " << dictionaryService->CheckWord("Tutorial") << std::endl;
    }
  }
}

#include <usModuleInitialization.h>

US_INITIALIZE_EXECUTABLE("main")
