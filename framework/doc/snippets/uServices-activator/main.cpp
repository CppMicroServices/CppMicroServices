#include "cppmicroservices/BundleActivator.h"

using namespace cppmicroservices;

//! [0]
class MyActivator : public BundleActivator
{

public:

  void Start(BundleContext /*context*/)
  { /* register stuff */ }


  void Stop(BundleContext /*context*/)
  { /* cleanup */ }

};

US_EXPORT_BUNDLE_ACTIVATOR(MyActivator)
//![0]

int main(int /*argc*/, char* /*argv*/[])
{
  MyActivator ma;
  return 0;
}
