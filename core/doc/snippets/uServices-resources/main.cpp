
#include <usGetBundleContext.h>
#include <usBundleContext.h>
#include <usBundleResource.h>
#include <usBundle.h>
#include <usBundleResourceStream.h>

#include <iostream>

using namespace us;

void resourceExample()
{
  //! [1]
  // Get this bundle's Bundle object
  Bundle* bundle = GetBundleContext()->GetBundle();

  BundleResource resource = bundle->GetResource("config.properties");
  if (resource.IsValid())
  {
    // Create a BundleResourceStream object
    BundleResourceStream resourceStream(resource);

    // Read the contents line by line
    std::string line;
    while (std::getline(resourceStream, line))
    {
      // Process the content
      std::cout << line << std::endl;
    }
  }
  else
  {
    // Error handling
  }
  //! [1]
}

void parseComponentDefinition(std::istream&)
{
}

void extenderPattern(BundleContext* bundleCtx)
{
  //! [2]
  // Get all installed bundles
  std::vector<Bundle*> bundles = bundleCtx->GetBundles();

  // Check if a bundle defines a "service-component" property
  // and use its value to retrieve an embedded resource containing
  // a component description.
  for(std::size_t i = 0; i < bundles.size(); ++i)
  {
    Bundle* const bundle = bundles[i];
    std::string componentPath = bundle->GetProperty("service-component").ToString();
    if (!componentPath.empty())
    {
      BundleResource componentResource = bundle->GetResource(componentPath);
      if (!componentResource.IsValid() || componentResource.IsDir()) continue;

      // Create a std::istream compatible object and parse the
      // component description.
      BundleResourceStream resStream(componentResource);
      parseComponentDefinition(resStream);
    }
  }
  //! [2]
}

int main(int /*argc*/, char* /*argv*/[])
{
  //! [0]
  BundleContext* bundleContext = GetBundleContext();
  Bundle* bundle = bundleContext->GetBundle();

  // List all XML files in the config directory
  std::vector<BundleResource> xmlFiles = bundle->FindResources("config", "*.xml", false);

  // Find the resource named vertex_shader.txt starting at the root directory
  std::vector<BundleResource> shaders = bundle->FindResources("", "vertex_shader.txt", true);
  //! [0]

  return 0;
}
