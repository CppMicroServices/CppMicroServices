#include <usModuleActivator.h>

//! [GetModuleContext]
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>

US_USE_NAMESPACE

void RetrieveModuleContext()
{
  ModuleContext* context = GetModuleContext();
  Module* module = context->GetModule();
  std::cout << "Module name: " << module->GetName() << " [id: " << module->GetModuleId() << "]\n";
}
//! [GetModuleContext]

//! [InitializeModule]
#include <usModuleInitialization.h>

US_INITIALIZE_MODULE
//! [InitializeModule]

int main(int /*argc*/, char* /*argv*/[])
{
  RetrieveModuleContext();
  return 0;
}
