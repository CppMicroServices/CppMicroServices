#include "ServiceImpl.hpp"
#include "cppmicroservices/Bundle.h"
#include <iostream>

namespace sample
{

    void
    ServiceComponentCA06::Updated(cppmicroservices::AnyMap const& properties)
    {
        auto frameworkCtx
            = cppmicroservices::any_cast<std::shared_ptr<cppmicroservices::BundleContext>>(properties.at("context"));
        std::vector<cppmicroservices::Bundle> bundles;
        std::string libName = "TestBundleDSCA02";
#if defined(US_BUILD_SHARED_LIBS)
        auto libPath = cppmicroservices::any_cast<std::string>(properties.at("libPath"));
        auto dirSep = cppmicroservices::any_cast<std::string>(properties.at("dirSep"));
        auto usLibPrefix = cppmicroservices::any_cast<std::string>(properties.at("usLibPrefix"));
        auto usLibPostfix = cppmicroservices::any_cast<std::string>(properties.at("usLibPostfix"));
        auto usLibExt = cppmicroservices::any_cast<std::string>(properties.at("usLibExt"));
        bundles = frameworkCtx->InstallBundles(libPath + dirSep + usLibPrefix + libName + usLibPostfix + usLibExt);
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
    }
} // namespace sample
