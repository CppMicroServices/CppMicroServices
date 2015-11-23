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

#include <usGetBundleContext.h>
#include <usBundleContext.h>

#include <vector>
#include <thread>

using namespace us;

namespace
{
    // Attempt to get as close an approximation as to how long it takes to install a bundle
    // without having the extra machinery of error handling in the way.
    inline void InstallTestBundleNoErrorHandling(BundleContext* frameworkCtx, const std::string& bundleName)
    {
#if defined (US_BUILD_SHARED_LIBS)
        frameworkCtx->InstallBundle(LIB_PATH + DIR_SEP + LIB_PREFIX + bundleName + LIB_EXT + "/" + bundleName);
#else
        frameworkCtx->InstallBundle(BIN_PATH + DIR_SEP + "usCoreTestDriver" + EXE_EXT + "/" + bundleName);
#endif
    }

    void TestSerial(std::shared_ptr<Framework> f)
    {
        // Installing such a small set of bundles doesn't yield significant
        // data about performance. Consider increasing the number of bundles
        // used.
        BundleContext* fmc = f->GetBundleContext();

        HighPrecisionTimer timer;
        timer.Start();
        InstallTestBundleNoErrorHandling(fmc, "TestBundleA");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleA2");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleB");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleH");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleM");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleR");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleRA");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleRL");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleS");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleSL1");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleSL3");
        InstallTestBundleNoErrorHandling(fmc, "TestBundleSL4");

        long long elapsedTimeInMilliSeconds = timer.ElapsedMilli();
        US_TEST_OUTPUT(<< "[thread " << std::this_thread::get_id() << "] Time elapsed to install 12 new bundles: " << elapsedTimeInMilliSeconds << " milliseconds");

        elapsedTimeInMilliSeconds = 0;

        auto bundles = f->GetBundleContext()->GetBundles();
        for (auto const& bundle : bundles)
        {
            timer.Start();
            bundle->Start();
            elapsedTimeInMilliSeconds += timer.ElapsedMilli();
        }
        
        US_TEST_OUTPUT(<< "[thread " << std::this_thread::get_id() << "] Time elapsed to start 12 bundles: " << elapsedTimeInMilliSeconds << " milliseconds");
    }

#ifdef US_ENABLE_THREADING_SUPPORT
    void TestConcurrent(const std::shared_ptr<Framework>& f)
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
                                                f->GetBundleContext()->GetBundles();
                                            }));
        }

        for (auto& th : threads) th.join();
        US_TEST_CONDITION(numTestBundles == f->GetBundleContext()->GetBundles().size(), "Test for correct number of installed bundles")
    }
#endif

}   // end anonymous namespace

int usBundleRegistryPerformanceTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("BundleRegistryPerformanceTest")

    FrameworkFactory factory;
    std::shared_ptr<Framework> framework = factory.NewFramework();
    framework->Start();

    // auto-installing will skew the benchmark results.
    framework->SetAutoLoadingEnabled(false);

    US_TEST_OUTPUT(<< "Testing serial installation of bundles");
    TestSerial(framework);

    for (auto& bundle : framework->GetBundleContext()->GetBundles())
    {
        if (bundle->GetBundleId() != 0)
        {
            bundle->Uninstall();
        }
    }
#ifdef US_ENABLE_THREADING_SUPPORT
    US_TEST_OUTPUT(<< "Testing concurrent installation of bundles");
    TestConcurrent(framework);
#endif

    framework->Stop();

    US_TEST_END()
}
