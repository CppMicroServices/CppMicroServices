#include <usGlobalConfig.h>

US_BEGIN_NAMESPACE
struct ModuleActivator;
US_END_NAMESPACE

US_USE_NAMESPACE

// This is just for illustration purposes in code snippets
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule1() { return 0; }
extern "C" ModuleActivator* _us_module_activator_instance_MyStaticModule2() { return 0; }
extern "C" ModuleActivator* _us_init_resources_MyStaticModule2() { return 0; }
void _us_import_module_initializer_MyStaticModule1() {}
void _us_import_module_initializer_MyStaticModule2() {}
void _us_import_module_initializer_CppMicroServices() {}

//! [ImportStaticModuleIntoMain]
#include <usModuleImport.h>

US_IMPORT_MODULE(MyStaticModule1)
//! [ImportStaticModuleIntoMain]

//! [ImportStaticModuleIntoMain2]
#include <usModuleImport.h>

US_IMPORT_MODULE(CppMicroServices)
US_IMPORT_MODULE(MyStaticModule2)
US_IMPORT_MODULE_RESOURCES(MyStaticModule2)
US_INITIALIZE_STATIC_MODULE(main)
//! [ImportStaticModuleIntoMain2]

int main(int /*argc*/, char* /*argv*/[])
{
  return 0;
}

//! [InitializeExecutable]
#include <usModuleInitialization.h>
US_INITIALIZE_MODULE
//! [InitializeExecutable]
