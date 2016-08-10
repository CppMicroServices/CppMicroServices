//! [GetBundleContext]
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/GetBundleContext.h"

#include <iostream>

using namespace cppmicroservices;

void RetrieveBundleContext()
{
  auto context = GetBundleContext();
  auto bundle = context.GetBundle();
  std::cout << "Bundle name: " << bundle.GetSymbolicName() << " [id: " << bundle.GetBundleId() << "]\n";
}
//! [GetBundleContext]

//! [InitializeBundle]
#include "cppmicroservices/BundleInitialization.h"

US_INITIALIZE_BUNDLE
//! [InitializeBundle]

int main(int /*argc*/, char* /*argv*/[])
{
  RetrieveBundleContext();
  return 0;
}
