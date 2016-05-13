#include <usBundleActivator.h>

//! [GetBundleContext]
#include <usGetBundleContext.h>
#include <usBundle.h>
#include <usBundleContext.h>

#include <iostream>

using namespace us;

void RetrieveBundleContext()
{
  auto context = GetBundleContext();
  auto bundle = context.GetBundle();
  std::cout << "Bundle name: " << bundle.GetSymbolicName() << " [id: " << bundle.GetBundleId() << "]\n";
}
//! [GetBundleContext]

//! [InitializeBundle]
#include <usBundleInitialization.h>

US_INITIALIZE_BUNDLE
//! [InitializeBundle]

int main(int /*argc*/, char* /*argv*/[])
{
  RetrieveBundleContext();
  return 0;
}
