#include <usBundleActivator.h>

//! [GetBundleContext]
#include <usGetBundleContext.h>
#include <usBundle.h>
#include <usBundleContext.h>

#include <iostream>

using namespace us;

void RetrieveBundleContext()
{
  BundleContext* context = GetBundleContext();
  Bundle* bundle = context->GetBundle();
  std::cout << "Bundle name: " << bundle->GetName() << " [id: " << bundle->GetBundleId() << "]\n";
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
