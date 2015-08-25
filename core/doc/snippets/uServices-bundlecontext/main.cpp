#include <usBundleActivator.h>

//! [GetBundleContext]
#include <usGetBundleContext.h>
#include <usBundle.h>
#include <usBundleContext.h>

US_USE_NAMESPACE

void RetrieveBundleContext()
{
  BundleContext* context = GetBundleContext();
  Bundle* bundle = context->GetBundle();
  std::cout << "Bundle name: " << bundle->GetName() << " [id: " << bundle->GetBundleId() << "]\n";
}
//! [GetBundleContext]

//! [InitializeBundle]

//! [InitializeBundle]

int main(int /*argc*/, char* /*argv*/[])
{
  RetrieveBundleContext();
  return 0;
}
