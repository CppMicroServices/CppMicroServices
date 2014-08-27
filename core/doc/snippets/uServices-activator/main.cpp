#include <usModuleActivator.h>

US_USE_NAMESPACE

//! [0]
class MyActivator : public ModuleActivator
{

public:

  void Load(ModuleContext* /*context*/)
  { /* register stuff */ }


  void Unload(ModuleContext* /*context*/)
  { /* cleanup */ }

};

US_EXPORT_MODULE_ACTIVATOR(MyActivator)
//![0]

int main(int /*argc*/, char* /*argv*/[])
{
  MyActivator ma;
  return 0;
}
