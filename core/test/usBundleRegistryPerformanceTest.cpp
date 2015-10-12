/*=============================================================================

Library: CppMicroServices

Copyright (c) The CppMicroServices developers. See the COPYRIGHT
file at the top-level directory of this distribution and at
https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=============================================================================*/

#include <usFrameworkFactory.h>
#include <usFramework.h>

#include "usTestUtils.h"
#include "usTestingConfig.h"
#include "usTestingMacros.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <vector>
#include <thread>

using namespace us;

namespace
{
    // Attempt to get as close an approximation as to how long it takes to install a bundle
    // without having the extra machinery of error handling in the way.
    inline void InstallTestBundleNoErrorHandling(ModuleContext* frameworkCtx, const std::string& bundleName)
    {
#if defined (US_BUILD_SHARED_LIBS)
        frameworkCtx->InstallBundle(LIB_PATH + DIR_SEP + LIB_PREFIX + bundleName + LIB_EXT + "/" + bundleName);
#else
        frameworkCtx->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/" + bundleName);
#endif
    }

    void TestSerial(Framework* f)
    {
        // Installing such a small set of bundles doesn't yield significant
        // data about performance. Consider increasing the number of bundles
        // used.
        ModuleContext* fmc = f->GetModuleContext();

        HighPrecisionTimer timer;
        timer.Start();
        InstallTestBundleNoErrorHandling(fmc, "TestModuleA");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleA2");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleB");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleH");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleM");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleR");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleRA");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleRL");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleS");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleSL1");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleSL3");
        InstallTestBundleNoErrorHandling(fmc, "TestModuleSL4");

        long long elapsedTimeInMilliSeconds = timer.ElapsedMilli();
        US_TEST_OUTPUT(<< "[thread " << std::this_thread::get_id() << "] Time elapsed to install 12 new bundles: " << elapsedTimeInMilliSeconds << " milliseconds");

        elapsedTimeInMilliSeconds = 0;

        std::vector<Module*> modules(f->GetModuleContext()->GetModules());
        for (auto bundle : modules)
        {
            timer.Start();
            bundle->Start();
            elapsedTimeInMilliSeconds += timer.ElapsedMilli();
        }
        
        US_TEST_OUTPUT(<< "[thread " << std::this_thread::get_id() << "] Time elapsed to start 12 bundles: " << elapsedTimeInMilliSeconds << " milliseconds");
    }

#ifdef US_ENABLE_THREADING_SUPPORT
    void TestConcurrent(Framework* f)
    {
        // This is by no means a "real world" example. At best it is a simulation to test
        // the performance of concurrent access to the bundle registry.
        // At any point in which real customer usage in a concurrent way becomes known,
        // it would be ideal to model it as a test.

        const std::size_t numTestBundles = 13;  // 12 test bundles + the system bundle
        const int numTestThreads = 100;
        std::vector<std::thread> threads;
        for (int i = 0; i < numTestThreads; ++i)
        {
            threads.push_back(std::thread(TestSerial, f));
            threads.push_back(std::thread([f]() -> void 
                                            {
                                                f->GetModuleContext()->GetModules();
                                            }));
        }

        for (auto& th : threads) th.join();
        US_TEST_CONDITION(numTestBundles == f->GetModuleContext()->GetModules().size(), "Test for correct number of installed bundles")
    }
#endif

}   // end anonymous namespace

int usBundleRegistryPerformanceTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("BundleRegistryPerformanceTest")

    FrameworkFactory factory;
    Framework* framework = factory.NewFramework();
    framework->Start();

    // auto-installing will skew the benchmark results.
    framework->SetAutoLoadingEnabled(false);

    US_TEST_OUTPUT(<< "Testing serial installation of bundles");
    TestSerial(framework);

    for (auto module : framework->GetModuleContext()->GetModules())
    {
        if (module->GetModuleId() != 1)
        {
            module->Uninstall();
        }
    }
#ifdef US_ENABLE_THREADING_SUPPORT
    US_TEST_OUTPUT(<< "Testing concurrent installation of bundles");
    TestConcurrent(framework);
#endif

    framework->Stop();
    delete framework;

    US_TEST_END()
}
