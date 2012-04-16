#include <usModuleActivator.h>

//! [InitializeModule]
#include <usModuleInitialization.h>

US_INITIALIZE_MODULE("My Module", "mylibname", "", "1.0.0")
//! [InitializeModule]

//! [GetModuleContext]
#include <usGetModuleContext.h>
#include <usModule.h>

US_USE_NAMESPACE

void RetrieveModuleContext()
{
  ModuleContext* context = GetModuleContext();
  Module* module = context->GetModule();
  std::cout << "Module name: " << module->GetName() << " [id: " << module->GetModuleId() << "]\n";
}
//! [GetModuleContext]

int main(int /*argc*/, char* /*argv*/[])
{
  RetrieveModuleContext();
  return 0;
}
