#include "ServiceImpl.hpp"
#include <iostream>

namespace sample
{

    void
    ServiceComponentCA06::Modified(std::shared_ptr<ComponentContext> const& context,
                                   std::shared_ptr<cppmicroservices::AnyMap> const& configuration)
    {
        auto frameworkCtx = context->GetBundleContext();
        std::vector<cppmicroservices::Bundle> bundles;
        std::string libName = "TestBundleDSCA02";
#if defined(US_BUILD_SHARED_LIBS)
        auto libPath = cppmicroservices::any_cast<std::string>((*configuration)["libPath"]);
        auto dirSep = cppmicroservices::any_cast<std::string>((*configuration)["dirSep"]);
        auto usLibPrefix = cppmicroservices::any_cast<std::string>((*configuration)["usLibPrefix"]);
        auto usLibPostfix = cppmicroservices::any_cast<std::string>((*configuration)["usLibPostfix"]);
        auto usLibExt = cppmicroservices::any_cast<std::string>((*configuration)["usLibExt"]);
        bundles = frameworkCtx.InstallBundles(libPath + dirSep + usLibPrefix + libName + usLibPostfix + usLibExt);
#else
        bundles = frameworkCtx.GetBundles();
#endif

        for (auto b : bundles)
        {
            if (b.GetSymbolicName() == libName)
            {
                b.Start();
            }
        }

        std::lock_guard<std::mutex> lock(propertiesLock);
        properties = configuration;
    }

    cppmicroservices::AnyMap
    ServiceComponentCA06::GetProperties()
    {
        std::lock_guard<std::mutex> lock(propertiesLock);
        return *properties;
    }

} // namespace sample
