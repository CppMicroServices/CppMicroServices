#include <usGlobalConfig.h>

namespace us {
struct BundleActivator;
class BundleContext;
}

using namespace us;

// This is just for illustration purposes in code snippets
extern "C" BundleActivator* _us_create_activator_MyStaticBundle1() { return nullptr; }
extern "C" BundleActivator* _us_create_activator_MyStaticBundle2() { return nullptr; }
extern "C" void _us_import_bundle_initializer_MyStaticBundle1() {}
extern "C" void _us_import_bundle_initializer_MyStaticBundle2() {}
extern "C" BundleContext* _us_get_bundle_context_instance_MyStaticBundle1() { return nullptr; }
extern "C" BundleContext* _us_set_bundle_context_instance_MyStaticBundle1() { return nullptr; }
extern "C" BundleContext* _us_get_bundle_context_instance_MyStaticBundle2() { return nullptr; }
extern "C" BundleContext* _us_set_bundle_context_instance_MyStaticBundle2() { return nullptr; }
extern "C" BundleContext* _us_get_bundle_context_instance_main() { return nullptr; }
extern "C" BundleContext* _us_set_bundle_context_instance_main() { return nullptr; }

//! [ImportStaticBundleIntoMain]
#include <usBundleImport.h>

US_IMPORT_BUNDLE(MyStaticBundle1)
//! [ImportStaticBundleIntoMain]

//! [ImportStaticBundleIntoMain2]
#include <usBundleImport.h>

#ifndef US_BUILD_SHARED_LIBS
US_INITIALIZE_STATIC_BUNDLE(system_bundle)
US_IMPORT_BUNDLE(MyStaticBundle2)
US_INITIALIZE_STATIC_BUNDLE(main)
#endif
//! [ImportStaticBundleIntoMain2]

int main(int /*argc*/, char* /*argv*/[])
{
  return 0;
}
