#include <usGlobalConfig.h>

US_BEGIN_NAMESPACE
struct ModuleActivator;
US_END_NAMESPACE

US_USE_NAMESPACE

// This is just for illustration purposes in code snippets
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule1() { return 0; }
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule2() { return 0; }
extern "C" void _us_import_module_initializer_MyStaticModule1() {}
extern "C" void _us_import_module_initializer_MyStaticModule2() {}

//! [ImportStaticModuleIntoMain]
#include <usModuleImport.h>

US_IMPORT_MODULE(MyStaticModule1)
//! [ImportStaticModuleIntoMain]

//! [ImportStaticModuleIntoMain2]
#include <usModuleImport.h>

#ifndef US_BUILD_SHARED_LIBS
US_IMPORT_MODULE(CppMicroServices)
US_IMPORT_MODULE(MyStaticModule2)
US_INITIALIZE_STATIC_MODULE(main)
#endif
//! [ImportStaticModuleIntoMain2]

int main(int /*argc*/, char* /*argv*/[])
{
  return 0;
}

//! [InitializeExecutable]
#include <usModuleInitialization.h>
US_INITIALIZE_MODULE
//! [InitializeExecutable]
