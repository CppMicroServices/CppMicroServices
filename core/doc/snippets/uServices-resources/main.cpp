#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModule.h>
#include <usModuleResourceStream.h>
#include <usModuleRegistry.h>

US_USE_NAMESPACE

void resourceExample()
{
  //! [1]
  // Get this module's Module object
  Module* module = GetModuleContext()->GetModule();

  ModuleResource resource = module->GetResource("config.properties");
  if (resource.IsValid())
  {
    // Create a ModuleResourceStream object
    ModuleResourceStream resourceStream(resource);

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

void extenderPattern()
{
  //! [2]
  // Get all loaded modules
  std::vector<Module*> modules = ModuleRegistry::GetLoadedModules();

  // Check if a module defines a "service-component" property
  // and use its value to retrieve an embedded resource containing
  // a component description.
  for(std::size_t i = 0; i < modules.size(); ++i)
  {
    Module* const module = modules[i];
    std::string componentPath = module->GetProperty("service-component").ToString();
    if (!componentPath.empty())
    {
      ModuleResource componentResource = module->GetResource(componentPath);
      if (!componentResource.IsValid() || componentResource.IsDir()) continue;

      // Create a std::istream compatible object and parse the
      // component description.
      ModuleResourceStream resStream(componentResource);
      parseComponentDefinition(resStream);
    }
  }
  //! [2]
}

int main(int /*argc*/, char* /*argv*/[])
{
  //! [0]
  ModuleContext* moduleContext = GetModuleContext();
  Module* module = moduleContext->GetModule();

  // List all XML files in the config directory
  std::vector<ModuleResource> xmlFiles = module->FindResources("config", "*.xml", false);

  // Find the resource named vertex_shader.txt starting at the root directory
  std::vector<ModuleResource> shaders = module->FindResources("", "vertex_shader.txt", true);
  //! [0]

  return 0;
}

#include <usModuleInitialization.h>
US_INITIALIZE_MODULE
