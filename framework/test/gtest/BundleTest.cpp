/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/GetBundleContext.h"
#include "cppmicroservices/ListenerToken.h"
#include "cppmicroservices/ServiceEvent.h"

#include "cppmicroservices/util/FileSystem.h"
#include "cppmicroservices/util/String.h"

#include "FrameworkTestActivator.h"
#include "TestUtilBundleListener.h"
#include "TestUtilFrameworkListener.h"
#include "TestUtils.h"
#include "TestingConfig.h"
#include "Mocks.h"
#include "MockUtils.h"

#include <chrono>
#include <future>
#include <thread>

#include "gtest/gtest.h"

US_MSVC_PUSH_DISABLE_WARNING(4996)

using namespace cppmicroservices;
using namespace cppmicroservices::testing;
using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::AtLeast;

class BundleTest : public ::testing::Test
{
  protected:
    cppmicroservices::MockBundleStorageMemory* bundleStorage;
    cppmicroservices::MockedEnvironment mockEnv;
    cppmicroservices::Framework& framework;
    cppmicroservices::BundleContext context;

  public:
    BundleTest()
        : bundleStorage(new MockBundleStorageMemory())
        , mockEnv(MockedEnvironment(bundleStorage))
        , framework(mockEnv.framework) {}

    ~BundleTest() override = default;

    void
    SetUp() override
    {
        framework.Start();
        context = framework.GetBundleContext();
    }

    void
    TearDown() override
    {
        framework.Stop();
        framework.WaitForStop(std::chrono::milliseconds::zero());
    }
};

class FrameworkTestSuite
{

    TestBundleListener listener;

    BundleContext bc;
    Bundle bu;
    Bundle buExec;
    Bundle buA;

  public:
    FrameworkTestSuite(BundleContext const& bc) : bc(bc), bu(bc.GetBundle()) {}

    void
    setup()
    {
        bc.AddBundleListener(&listener, &TestBundleListener::BundleChanged);
        bc.AddServiceListener(&listener, &TestBundleListener::ServiceChanged);
    }

    void
    cleanup()
    {
        std::vector<Bundle> bundles = { buA, buExec };

        for (auto& b : bundles)
        {
            if (b)
            {
                b.Uninstall();
            }
        }

        buExec = nullptr;
        buA = nullptr;

        bc.RemoveBundleListener(&listener, &TestBundleListener::BundleChanged);
        bc.RemoveServiceListener(&listener, &TestBundleListener::ServiceChanged);
    }

    //----------------------------------------------------------------------------
    // Test result of GetService(ServiceReference()). Should throw std::invalid_argument
    void
    frame018a()
    {
        EXPECT_THROW(bc.GetService(ServiceReferenceU()), std::invalid_argument);
    }

    // Load libA and check that it exists and that its expected service does not exist,
    // Also check that the expected events in the framework occurs
    void
    frame020a()
    {
        buA = cppmicroservices::testing::InstallLib(bc, "TestBundleA");
        // Test for existing bundle TestBundleA
        ASSERT_TRUE(buA);
        ASSERT_EQ(buA.GetSymbolicName(), "TestBundleA");

        // Test bundle A in state installed
        ASSERT_EQ(buA.GetState(), Bundle::STATE_INSTALLED);

        // Test bundle A last modified
        ASSERT_TRUE(buA.GetLastModified() > Bundle::TimeStamp());
        // Test bundle A last modified
        ASSERT_TRUE(buA.GetLastModified() <= std::chrono::steady_clock::now());

        // Check that no service reference exist yet.
        ServiceReferenceU sr1 = bc.GetServiceReference("cppmicroservices::TestBundleAService");
        ASSERT_FALSE(sr1) << "service from bundle A must not exist yet";

        // Check manifest headers
        auto const& headers = buA.GetHeaders();
        EXPECT_GT(static_cast<int>(headers.size()), 0);
        ASSERT_EQ(headers.at("bundle.symbolic_name"), std::string("TestBundleA"));

        // check the listeners for events
        std::vector<BundleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS // The bundle is installed at framework startup for static builds
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, buA));
        bool relaxed = false;
#else
        bool relaxed = true;
#endif
        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, relaxed));
    }

    // Start libA and check that it gets state ACTIVE
    // and that the service it registers exist
    void
    frame025a()
    {
        buA.Start();
        // Test bundle A in state active
        ASSERT_EQ(buA.GetState(), Bundle::STATE_ACTIVE);

        // Check if testbundleA registered the expected service
        ServiceReferenceU sr1 = bc.GetServiceReference("cppmicroservices::TestBundleAService");
        ASSERT_TRUE(sr1);

        auto o1 = bc.GetService(sr1);
        ASSERT_TRUE(o1 && !o1->empty());

        // check the listeners for events
        std::vector<BundleEvent> pEvts;
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, buA));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, buA));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, buA));

        std::vector<ServiceEvent> seEvts;
        seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_REGISTERED, sr1));

        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, seEvts));
    }

    // Start libA and check that it exists and that the storage paths are correct
    void
    frame020b(std::string const& tempPath)
    {
        buA = cppmicroservices::testing::InstallLib(bc, "TestBundleA");
        // Test for existing bundle TestBundleA
        ASSERT_TRUE(buA);

        buA.Start();

        // Test for valid base storage path
        ASSERT_EQ(bc.GetProperty(Constants::FRAMEWORK_STORAGE).ToString(), tempPath);

        // launching properties should be accessible through any bundle
        // Test for valid base storage path
        ASSERT_EQ(buA.GetBundleContext().GetProperty(Constants::FRAMEWORK_STORAGE).ToString(), tempPath);

        const std::string baseStoragePath
            = tempPath + util::DIR_SEP + "data" + util::DIR_SEP + util::ToString(buA.GetBundleId()) + util::DIR_SEP;
        // Test for valid data path
        ASSERT_EQ(buA.GetBundleContext().GetDataFile(""), baseStoragePath);
        // Test for valid data file path
        ASSERT_EQ(buA.GetBundleContext().GetDataFile("bla"), (baseStoragePath + "bla"));

        // Test if started correctly
        ASSERT_TRUE(buA.GetState() & Bundle::STATE_ACTIVE);
    }

    // Stop libA and check for correct events
    void
    frame030b()
    {
        ServiceReferenceU sr1 = buA.GetBundleContext().GetServiceReference("cppmicroservices::TestBundleAService");
        // Test for valid service reference
        ASSERT_TRUE(sr1);

        auto lm = buA.GetLastModified();
        buA.Stop();
        // Test for stopped state
        ASSERT_NE(buA.GetState(), Bundle::STATE_ACTIVE);
        // Unchanged last modified time after stop
        ASSERT_EQ(lm, buA.GetLastModified());

        // Check manifest headers in stopped state
        auto const& headers = buA.GetHeaders();
        EXPECT_GT(static_cast<int>(headers.size()), 0);
        ASSERT_EQ(headers.at("bundle.symbolic_name"), std::string("TestBundleA"));

        std::vector<BundleEvent> pEvts;
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, buA));
        pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, buA));

        std::vector<ServiceEvent> seEvts;
        seEvts.push_back(ServiceEvent(ServiceEvent::SERVICE_UNREGISTERING, sr1));

        // Test for unexpected events
        ASSERT_TRUE(listener.CheckListenerEvents(pEvts, seEvts));
    }

    // Check that the executable's activator was started and called
    void
    frame035a()
    {
        buExec = cppmicroservices::testing::GetBundle("main", bc);
        ASSERT_TRUE(buExec);
        buExec.Start();

        // Test FrameworkTestActivator::Start() called for executable
        ASSERT_TRUE(FrameworkTestActivator::StartCalled());

        long systemId = 0;
        // check expected meta-data
        ASSERT_EQ("main", buExec.GetSymbolicName());
        // Test test driver bundle version
        ASSERT_EQ(BundleVersion(0, 1, 0), buExec.GetVersion());

        // Test CppMicroServices version
        ASSERT_EQ(BundleVersion(CppMicroServices_VERSION_MAJOR,
                                CppMicroServices_VERSION_MINOR,
                                CppMicroServices_VERSION_PATCH),
                  buExec.GetBundleContext().GetBundle(systemId).GetVersion());
    }

    // Get location, persistent storage and status of the bundle.
    // Test bundle context properties
    void
    frame037a()
    {
        std::string location = buExec.GetLocation();
        // Test for non-empty bundle location
        ASSERT_FALSE(location.empty());
        // Test for started flag
        ASSERT_EQ(buExec.GetState(), Bundle::STATE_ACTIVE);

        // launching properties should be accessible through any bundle
        auto p1 = bc.GetBundle().GetBundleContext().GetProperty(Constants::FRAMEWORK_UUID);
        auto p2 = buExec.GetBundleContext().GetProperty(Constants::FRAMEWORK_UUID);
        // Test for uuid accesible from framework and bundle
        ASSERT_FALSE(p1.Empty());
        ASSERT_EQ(p1.ToString(), p2.ToString());

        const std::string baseStoragePath = util::GetCurrentWorkingDirectory();

        // Test for valid data path
        ASSERT_EQ(buExec.GetBundleContext().GetDataFile("").substr(0, baseStoragePath.size()), baseStoragePath);
        // Test for valid data file path
        ASSERT_EQ(buExec.GetBundleContext().GetDataFile("bla").substr(0, baseStoragePath.size()), baseStoragePath);

        // Test for non-empty framework uuid property
        ASSERT_FALSE(buExec.GetBundleContext().GetProperty(Constants::FRAMEWORK_UUID).Empty());
        auto props = buExec.GetBundleContext().GetProperties();
        // Test for non-empty bundle props
        ASSERT_FALSE(props.empty());
        // Test for existing framework version prop
        ASSERT_EQ(props.count(Constants::FRAMEWORK_VERSION), 1);
    }

    struct LocalListener
    {
        void
        ServiceChanged(ServiceEvent const&)
        {
        }
    };

    // Add a service listener with a broken LDAP filter to Get an exception
    void
    frame045a()
    {
        LocalListener sListen1;
        std::string brokenFilter = "A broken LDAP filter";

        EXPECT_THROW(bc.AddServiceListener(&sListen1, &LocalListener::ServiceChanged, brokenFilter),
                     std::invalid_argument);
    }
};

TEST_F(BundleTest, TestFramework)
{
    FrameworkTestSuite ts(framework.GetBundleContext());

    ts.setup();

    ts.frame018a();

    ts.frame020a();
    ts.frame025a();
    ts.frame030b();

    ts.frame035a();
    ts.frame037a();
    ts.frame045a();

    ts.cleanup();
}

TEST_F(BundleTest, testNonDefaultFramework)
{
    cppmicroservices::testing::TempDir frameworkStorage = cppmicroservices::testing::MakeUniqueTempDirectory();
    FrameworkConfiguration frameworkConfig;
    frameworkConfig[Constants::FRAMEWORK_STORAGE] = static_cast<std::string>(frameworkStorage);
    auto framework = FrameworkFactory().NewFramework(frameworkConfig);
    framework.Start();

    FrameworkTestSuite ts(framework.GetBundleContext());
    ts.setup();
    ts.frame020b(frameworkStorage);
    ts.cleanup();
}

// Verify that the same member function pointers registered as listeners
// with different receivers works.
TEST_F(BundleTest, TestListenerFunctors)
{
    TestBundleListener listener1;
    TestBundleListener listener2;

    context.RemoveBundleListener(&listener1, &TestBundleListener::BundleChanged);
    context.AddBundleListener(&listener1, &TestBundleListener::BundleChanged);
    context.RemoveBundleListener(&listener2, &TestBundleListener::BundleChanged);
    context.AddBundleListener(&listener2, &TestBundleListener::BundleChanged);

    auto bundleA = InstallLib(context, "TestBundleA");
    // Test for existing bundle TestBundleA
    ASSERT_TRUE(bundleA);

    bundleA.Start();

    std::vector<BundleEvent> pEvts;
#ifdef US_BUILD_SHARED_LIBS
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundleA));
#endif
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundleA));
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundleA));
    pEvts.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundleA));

    std::vector<ServiceEvent> seEvts;

    bool relaxed = false;
#ifndef US_BUILD_SHARED_LIBS
    relaxed = true;
#endif
    // Check first bundle listener
    ASSERT_TRUE(listener1.CheckListenerEvents(pEvts, seEvts, relaxed));
    // Check second bundle listener
    ASSERT_TRUE(listener2.CheckListenerEvents(pEvts, seEvts, relaxed));

    context.RemoveBundleListener(&listener1, &TestBundleListener::BundleChanged);
    context.RemoveBundleListener(&listener2, &TestBundleListener::BundleChanged);

    bundleA.Stop();
}

TEST_F(BundleTest, TestBundleStates)
{
    TestBundleListener listener;
    std::vector<BundleEvent> bundleEvents;
    FrameworkFactory factory;

    FrameworkConfiguration frameworkConfig;
    auto framework = factory.NewFramework(frameworkConfig);
    framework.Start();

    auto frameworkCtx = framework.GetBundleContext();
    frameworkCtx.AddBundleListener(&listener, &TestBundleListener::BundleChanged);

    // Test Start -> Stop for auto-installed bundles
    auto bundles = frameworkCtx.GetBundles();
    for (auto& bundle : bundles)
    {
        // TestStartBundleA and TestStopBundleA start/stop TestBundleA for the purposes
        // of a different test. Instead of complicating this test code, skip starting
        // these bundles. In the future, refactor this test code to execute only for static
        // builds and to start and stop an explicit list of bundles instead of them all.
        if (bundle != framework && "TestStartBundleA" != bundle.GetSymbolicName()
            && "TestStopBundleA" != bundle.GetSymbolicName())
        {
            // Test installed bundle state
            ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);
            try
            {
                bundle.Start();
                // Test started bundle state
                ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
            }
            catch (std::runtime_error const& /*ex*/)
            {
                // Test bundle state if bundle start failed
                ASSERT_EQ(bundle.GetState(), Bundle::STATE_RESOLVED);
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
            }
            // stop the bundle if it is in active state
            if (bundle.GetState() & Bundle::STATE_ACTIVE)
            {
                bundle.Stop();
                // Test stopped bundle state
                ASSERT_NE(bundle.GetState(), Bundle::STATE_ACTIVE);
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
                bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
            }
        }
    }
    // Test for unexpected events
    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents, false));
    bundleEvents.clear();

#ifdef US_BUILD_SHARED_LIBS // following combination can be tested only for shared library builds
    Bundle bundle;
    // Test install -> uninstall
    // expect 2 event (BUNDLE_INSTALLED, BUNDLE_UNINSTALLED)
    bundle = InstallLib(frameworkCtx, "TestBundleA");
    // Test non-empty bundle
    ASSERT_TRUE(bundle);
    // Test installed bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);
    bundle.Uninstall();
    // Test bundle install -> uninstall
    ASSERT_FALSE(frameworkCtx.GetBundle(bundle.GetBundleId()));
    // Test uninstalled bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_UNINSTALLED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    // Test for unexpected events
    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents, false));
    bundleEvents.clear();

    // Test install -> start -> uninstall
    // expect 6 events (BUNDLE_INSTALLED, BUNDLE_STARTING, BUNDLE_STARTED, BUNDLE_STOPPING, BUNDLE_STOPPED,
    // BUNDLE_UNINSTALLED)
    bundle = InstallLib(frameworkCtx, "TestBundleA");
    // Test non-empty bundle
    ASSERT_TRUE(bundle);
    // Test installed bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);
    bundle.Start();
    // Test started bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);
    bundle.Uninstall();
    // Test uninstalled bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_UNINSTALLED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    // Test for unexpected events
    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents));
    bundleEvents.clear();

    // Test install -> stop -> uninstall
    // expect 2 event (BUNDLE_INSTALLED, BUNDLE_UNINSTALLED)
    bundle = InstallLib(frameworkCtx, "TestBundleA");
    // Test non-empty bundle
    ASSERT_TRUE(bundle);
    // Test installed bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);
    bundle.Stop();
    // Test stopped bundle state
    ASSERT_NE(bundle.GetState(), Bundle::STATE_ACTIVE);
    bundle.Uninstall();
    // Test uninstalled bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_UNINSTALLED);
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    // Test for unexpected events
    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents));
    bundleEvents.clear();

    // Test install -> start -> stop -> uninstall
    // expect 6 events (BUNDLE_INSTALLED, BUNDLE_STARTING, BUNDLE_STARTED, BUNDLE_STOPPING, BUNDLE_STOPPED,
    // BUNDLE_UNINSTALLED)
    bundle = InstallLib(frameworkCtx, "TestBundleA");
    auto lm = bundle.GetLastModified();
    // Test non-empty bundle
    ASSERT_TRUE(bundle);
    // Test installed bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_INSTALLED);
    bundle.Start();
    // Test started bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_ACTIVE);
    bundle.Stop();
    // Test stopped bundle state
    ASSERT_NE(bundle.GetState(), Bundle::STATE_ACTIVE);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    bundle.Uninstall();
    // Test uninstalled bundle state
    ASSERT_EQ(bundle.GetState(), Bundle::STATE_UNINSTALLED);
    // Last modified time changed after uninstall
    ASSERT_TRUE(lm < bundle.GetLastModified());
    ASSERT_TRUE(bundle.GetLastModified() <= std::chrono::steady_clock::now());
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_INSTALLED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_RESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STARTED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPING, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_STOPPED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNRESOLVED, bundle));
    bundleEvents.push_back(BundleEvent(BundleEvent::BUNDLE_UNINSTALLED, bundle));
    // Test for unexpected events
    ASSERT_TRUE(listener.CheckListenerEvents(bundleEvents));
    bundleEvents.clear();
#endif
}

TEST_F(BundleTest, TestForInstallFailure)
{
    // Test that bogus bundle installs throw the appropriate exception
    EXPECT_THROW(context.InstallBundles(std::string {}), std::runtime_error);
    EXPECT_THROW(context.InstallBundles(std::string("\\path\\which\\won't\\exist\\phantom_bundle")),
                 std::runtime_error);

#ifdef US_BUILD_SHARED_LIBS
    // 2 bundles - the framework(system_bundle) and the executable(main).
    ASSERT_EQ(2, context.GetBundles().size());
#else
    // There are atleast 2 bundles, maybe more depending on how the executable is created
    ASSERT_GE(static_cast<int>(context.GetBundles().size()), 2);
#endif
}

TEST_F(BundleTest, TestDuplicateInstall)
{
    // Test installing the same bundle (i.e. a bundle with the same location) twice.
    // The exact same bundle should be returned on the second install.
    auto bundle = InstallLib(context, "TestBundleA");
    auto bundleDuplicate = InstallLib(context, "TestBundleA");

    // Test for the same bundle instance
    ASSERT_EQ(bundle, bundleDuplicate);
    // Test for the same bundle id
    ASSERT_EQ(bundle.GetBundleId(), bundleDuplicate.GetBundleId());
}

TEST_F(BundleTest, TestAutoInstallEmbeddedBundles)
{
    context.InstallBundles(BIN_PATH + util::DIR_SEP + "usFrameworkTests" + US_EXE_EXT);

#ifdef US_BUILD_SHARED_LIBS
    // 2 bundles - the framework(system_bundle) and the executable(main).
    ASSERT_EQ(2, context.GetBundles().size());
#else
    // There are atleast 2 bundles, maybe more depending on how the executable is created
    ASSERT_GE(static_cast<int>(context.GetBundles().size()), 2);
#endif

    auto bundles = context.GetBundles();
    auto bundleIter = std::find_if(bundles.begin(),
                                   bundles.end(),
                                   [](Bundle const& b) { return (std::string("main") == b.GetSymbolicName()); });

    ASSERT_NE(bundleIter, bundles.end());
    (*bundleIter).Start();
    (*bundleIter).Uninstall();

    auto b = context.GetBundle(0);
    EXPECT_THROW(b.Uninstall(), std::runtime_error);
}

TEST_F(BundleTest, TestNonStandardBundleExtension)
{
#ifdef US_BUILD_SHARED_LIBS
    context.InstallBundles(LIB_PATH + util::DIR_SEP + US_LIB_PREFIX + "TestBundleExt" + US_LIB_POSTFIX + ".cppms");

    // 3 bundles - the framework(system_bundle), the executable(main) and TextBundleExt
    ASSERT_EQ(3, context.GetBundles().size());
#else
    // There are atleast 3 bundles, maybe more depending on how the executable is created
    EXPECT_GE(static_cast<int>(context.GetBundles().size()), 3);
#endif

    // Test the non-standard file extension bundle's lifecycle
    auto bundles = context.GetBundles();
    auto bundleIter
        = std::find_if(bundles.begin(),
                       bundles.end(),
                       [](Bundle const& b) { return (std::string("TestBundleExt") == b.GetSymbolicName()); });

    ASSERT_NE(bundleIter, bundles.end());
    (*bundleIter).Start();
    (*bundleIter).Uninstall();
}

TEST_F(BundleTest, TestUnicodePaths)
{
    std::string bundleName = "TestBundleU";
    std::string pathName = LIB_PATH + cppmicroservices::util::DIR_SEP + u8"くいりのまちとこしくそ"
                            + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + "TestBundleU" + US_LIB_POSTFIX
                            + US_LIB_EXT;
    AnyMap manifest = AnyMap({
        { "bundle.activator", Any(true) },
        { "bundle.symbolic_name", Any(std::string(bundleName)) },
        { "unicode.sample", Any(std::string(u8"电脑 くいりのまちとこしくそ")) }
    });

    std::vector<std::string> files = {"TestBundleU"};
    auto resCont = std::make_shared<MockBundleResourceContainer>();
    ON_CALL(*resCont, GetTopLevelDirs())
        .WillByDefault(Return(files));
    EXPECT_CALL(*resCont, GetTopLevelDirs()).Times(1);

    ON_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bundleStorage,
            resCont,
            bundleName, pathName, 1,
            manifest
        )));
    EXPECT_CALL(*bundleStorage, CreateAndInsertArchive(_, Eq(bundleName), _)).Times(1);

    auto bundles = mockEnv.Install(bundleName, manifest, resCont);
    ASSERT_EQ(1, bundles.size()) << "Mock bundle failed to install correctly.";
    auto bundle = bundles.at(0);

    // Bundle location is the same as the path used to install
    ASSERT_EQ(bundle.GetLocation(), pathName);
    bundle.Start();
    // Bundle check start state
    ASSERT_EQ(bundle.GetState(), Bundle::State::STATE_ACTIVE);
    bundle.Stop();
}

TEST_F(BundleTest, TestBundleStartOptions)
{
    auto bundle = InstallLib(context, "TestBundleA");
    EXPECT_NO_THROW(bundle.Start(cppmicroservices::Bundle::StartOptions::START_ACTIVATION_POLICY));
}

TEST_F(BundleTest, TestBundleLessThanOperator)
{

    auto bundleA = InstallLib(context, "TestBundleA");
    auto bundleB = InstallLib(context, "TestBundleB");

    // Test that bundles are ordered correctly.
    ASSERT_TRUE(bundleA < bundleB);
    ASSERT_TRUE(bundleA < Bundle());
    ASSERT_TRUE(!(Bundle() < bundleB));
    ASSERT_TRUE(!(Bundle() < Bundle()));
}

TEST_F(BundleTest, TestBundleAssignmentOperator)
{
    Bundle b;
    Bundle b1;
    b = b1;
    // Test that the bundle objects are equivalent.
    ASSERT_EQ(b1, b);

    auto bundleA = InstallLib(context, "TestBundleA");
    auto bundleB = InstallLib(context, "TestBundleB");

    // Test that the bundles are different before assignment.
    ASSERT_NE(bundleB.GetBundleId(), bundleA.GetBundleId());
    ASSERT_NE(bundleB, bundleA);

    bundleB = bundleA;

    // Test bundle object assignment.
    ASSERT_EQ(bundleB.GetBundleId(), bundleA.GetBundleId());
    // Test that the bundle objects are equivalent.
    ASSERT_EQ(bundleB, bundleA);
}

TEST_F(BundleTest, TestBundleGetProperties)
{
    auto bundle = InstallLib(context, "TestBundleA");
    auto bundleProperties = bundle.GetProperties();

    // Test that bundle properties exist
    ASSERT_FALSE(bundleProperties.empty());

    Any symbolicNameProperty(bundleProperties.at(Constants::BUNDLE_SYMBOLICNAME));
    // Test that bundle properties contain an expected property.
    ASSERT_FALSE(symbolicNameProperty.Empty());
    ASSERT_EQ("TestBundleA", symbolicNameProperty.ToStringNoExcept());
}

TEST_F(BundleTest, TestBundleGetProperty)
{
    auto bundle = InstallLib(context, "TestBundleA");

    // get a bundle property
    Any bundleProperty(bundle.GetProperty(Constants::BUNDLE_SYMBOLICNAME));
    // Test querying a bundle property succeeds.
    ASSERT_EQ("TestBundleA", bundleProperty.ToStringNoExcept());

    // get a framework property
    Any frameworkProperty(bundle.GetProperty(Constants::FRAMEWORK_THREADING_SUPPORT));
    // Test that querying a framework property from a bundle object succeeds.
    ASSERT_FALSE(frameworkProperty.ToStringNoExcept().empty());

    // get a non existent property
    Any emptyProperty(bundle.GetProperty("does.not.exist"));
    // Test that querying a non-existent property returns an empty property object.
    ASSERT_TRUE(emptyProperty.Empty());
}

TEST_F(BundleTest, TestBundleGetPropertyKeys)
{
    auto bundle = InstallLib(context, "TestBundleA");
    auto keys = bundle.GetPropertyKeys();

    // Test that bundle property keys exist
    ASSERT_FALSE(keys.empty());

    // Test that property keys contain the expected keys.
    ASSERT_NE(keys.end(), std::find(keys.begin(), keys.end(), Constants::BUNDLE_SYMBOLICNAME));
    ASSERT_NE(keys.end(), std::find(keys.begin(), keys.end(), Constants::BUNDLE_ACTIVATOR));
}

#if defined(US_BUILD_SHARED_LIBS)
// Test installing a bundle with invalid meta-data in its manifest.
TEST_F(BundleTest, TestBundleManifestFailures)
{
    MockCoreBundleContext cbc;
    MockBundleStorageMemory bsm;

    cbc.storage = std::make_unique<MockBundleStorageMemory>();
    
    // TODO: Pull out MockBundleArchive constructor into helper function

    // throw if manifest.json bundle version key is not a string type
    std::shared_ptr<MockBundleArchive> ba = std::make_shared<MockBundleArchive>(
        &bsm,
        std::make_shared<MockBundleResourceContainer>(),
        "", "", 0,
        AnyMap({
            { "bundle.symbolic_name", Any(std::string("TestBundleMWithInvalidVersionType")) },
            { "bundle.description", Any(std::string("A test bundle containing an incorrect bundle.version value type.")) },
            { "bundle.version", Any(1.0) },
            { "bundle.activator", Any(true) }
        })
    );
    EXPECT_THROW(BundlePrivate(&cbc, ba), std::invalid_argument);

    // throw if BundleVersion ctor throws in BundlePrivate ctor
    ba = std::make_shared<MockBundleArchive>(
        &bsm,
        std::make_shared<MockBundleResourceContainer>(),
        "", "", 0,
        AnyMap({
            { "bundle.symbolic_name", Any(std::string("TestBundleMWithInvalidVersion")) },
            { "bundle.description", Any(std::string("A test bundle containing an invalid bundle.version value.")) },
            { "bundle.version", Any(std::string("0.0.1.-??0??")) },
            { "bundle.activator", Any(true) }
        })
    );
    EXPECT_THROW(BundlePrivate(&cbc, ba), std::invalid_argument);

    // throw if missing bundle.symbolic_name key in manifest.json
    ba = std::make_shared<MockBundleArchive>(
        &bsm,
        std::make_shared<MockBundleResourceContainer>(),
        "", "", 0,
        AnyMap({
            { "bundle.description", Any(std::string("A test bundle missing bundle.symbolic_name.")) },
            { "bundle.version", Any(std::string("1.0")) },
            { "bundle.activator", Any(true) }
        })
    );
    EXPECT_THROW(BundlePrivate(&cbc, ba), std::invalid_argument);

    // throw if empty bundle.symbolic_name value in manifest.json
    ba = std::make_shared<MockBundleArchive>(
        &bsm,
        std::make_shared<MockBundleResourceContainer>(),
        "", "", 0,
        AnyMap({
            { "bundle.symbolic_name", Any(std::string("")) },
            { "bundle.description", Any(std::string("A test bundle containing an empty bundle.symbolic_name value.")) },
            { "bundle.version", Any(std::string("1.0")) },
            { "bundle.activator", Any(true) }
        })
    );
    EXPECT_THROW(BundlePrivate(&cbc, ba), std::invalid_argument);
}
#endif

// Test the behavior of illegal bundle state changes.
TEST_F(BundleTest, TestIllegalBundleStateChange)
{
    /*
    MockCoreBundleContext cbc;
    MockBundleStorageMemory* bsm = new MockBundleStorageMemory();

    ON_CALL(*bsm, CreateAndInsertArchive(_, _, _))
        .WillByDefault(Return(std::make_shared<MockBundleArchive>(
            bsm,
            std::make_shared<MockBundleResourceContainer>(),
            "MOCK", "TestBundleA", 1,
            AnyMap({
                { "bundle.symbolic_name", Any(std::string("TestBundleA")) },
                { "bundle.description", Any(std::string("A test bundle missing bundle.symbolic_name.")) },
                { "bundle.version", Any(std::string("1.0")) },
                { "bundle.activator", Any(true) }
            })
        )));
    ON_CALL(*bsm, RemoveArchive(_)).WillByDefault(Return(true));
    EXPECT_CALL(*bsm, CreateAndInsertArchive(_, _, _)).Times(1);
    // EXPECT_CALL(*bsm, RemoveArchive(_)).Times(1);

    cbc.storage = std::unique_ptr<MockBundleStorageMemory>(bsm);

    std::shared_ptr<BundlePrivate> bp = std::make_shared<BundlePrivate>(&cbc);
    std::shared_ptr<BundleContextPrivate> bcp = std::make_shared<BundleContextPrivate>(bp.get());
    BundleContext bc = MakeBundleContext(bcp);

    auto bundleA = InstallLib(bc, "TestBundleA");

    // TODO: This fails because of invalid bundle (coreCtx is nullptr within BundleRegistry::Install0)
    bundleA.Start();
    bundleA.Uninstall();
    // Test stopping an uninstalled bundle
    EXPECT_THROW(bundleA.Stop(), std::logic_error);
    */

    auto bundleA = InstallLib(context, "TestBundleA");
    bundleA.Start();
    bundleA.Uninstall();
    // Test stopping an uninstalled bundle
    EXPECT_THROW(bundleA.Stop(), std::logic_error);
    framework.Stop();
    framework.WaitForStop(std::chrono::seconds::zero());

    framework = FrameworkFactory().NewFramework();
    framework.Start();
    context = framework.GetBundleContext();

    bundleA = InstallLib(context, "TestBundleA");
    bundleA.Start();
    bundleA.Uninstall();
    // Test starting an uninstalled bundle
    EXPECT_THROW(bundleA.Start(), std::logic_error);

    framework.Stop();
    framework.WaitForStop(std::chrono::seconds::zero());

    framework = FrameworkFactory().NewFramework();
    framework.Start();
    context = framework.GetBundleContext();

    bundleA = InstallLib(context, "TestBundleA");
    bundleA.Start();
    bundleA.Uninstall();
    // Test uninstalling an already uninstalled bundle should throw
    EXPECT_THROW(bundleA.Uninstall(), std::logic_error);

    // Test that BundlePrivate::CheckUninstalled throws on bundle operations performed after its been uninstalled.
    EXPECT_THROW(bundleA.GetRegisteredServices(), std::logic_error);

    // Test that installing a bundle with the same symbolic name and version throws an exception.
#if defined(US_BUILD_SHARED_LIBS)
    InstallLib(context, "TestBundleA");
    EXPECT_THROW(InstallLib(context, "TestBundleADuplicate"), std::runtime_error);
#endif
}

#if defined(US_BUILD_SHARED_LIBS)
// Test the behavior of a bundle activator which throws an exception.
TEST_F(BundleTest, TestBundleActivatorFailures)
{
    TestFrameworkListener listener;
    auto token = context.AddFrameworkListener(
        std::bind(&TestFrameworkListener::frameworkEvent, &listener, std::placeholders::_1));

    auto bundleStopFail = InstallLib(context, "TestBundleStopFail");
    bundleStopFail.Start();
    // Test the state of a bundle after stop failed
    EXPECT_THROW(bundleStopFail.Stop(), std::runtime_error);
    // Test that the bundle is not active after a failed stop.
    ASSERT_EQ(Bundle::State::STATE_RESOLVED, bundleStopFail.GetState());

    bundleStopFail.Start();
    // Test that the bundle is active prior to uninstall.
    ASSERT_EQ(Bundle::State::STATE_ACTIVE, bundleStopFail.GetState());
    // Test that bundle stop throws an exception and is sent as a FrameworkEvent during uninstall
    bundleStopFail.Uninstall();
    // Test that one FrameworkEvent was received.
    ASSERT_EQ(1, listener.events_received());
    // Test that the correct FrameworkEvent was received
    ASSERT_TRUE(listener.CheckEvents(std::vector<FrameworkEvent> {
        FrameworkEvent {FrameworkEvent::Type::FRAMEWORK_ERROR,
                        bundleStopFail, std::string(),
                        std::make_exception_ptr(std::runtime_error("whoopsie!"))}
    }));
    context.RemoveListener(std::move(token));
    // Test that even if Stop throws, the bundle is uninstalled.
    ASSERT_EQ(Bundle::State::STATE_UNINSTALLED, bundleStopFail.GetState());

    auto bundleStartFail = InstallLib(context, "TestBundleStartFail");
    // Test the state of a bundle after start failed
    EXPECT_THROW(bundleStartFail.Start(), std::runtime_error);
    // Test that the bundle is not active after a failed start
    ASSERT_EQ(Bundle::State::STATE_RESOLVED, bundleStartFail.GetState());
}
#endif

TEST_F(BundleTest, TestBundleStreamOperator)
{
    auto const bundle = InstallLib(context, "TestBundleA");
    ASSERT_TRUE(bundle);
    std::cout << &bundle;
}

#if defined(US_BUILD_SHARED_LIBS) && defined(US_ENABLE_THREADING_SUPPORT)
// This test is designed to be run in a loop to find race conditions
// when shutting down the framework when other framework operations are
// happening.
TEST_F(BundleTest, TestFrameworkAccessDuringFrameworkShutdown)
{

    std::promise<void> frameworkshuttingdown;
    std::future<void> waitForShuttingDown = frameworkshuttingdown.get_future();

    std::atomic_bool keepLooping = true;

    auto frameworkAccessThread
        = std::async(std::launch::async,
                     [this, &keepLooping, waitForIt = std::move(waitForShuttingDown)]()
                     {
                         waitForIt.wait();

                         while (keepLooping)
                         {
                             auto token = framework.GetBundleContext().AddBundleListener(
                                 [](const cppmicroservices::BundleEvent& evt)
                                 {
                                     if (evt.GetType() == cppmicroservices::BundleEvent::Type::BUNDLE_STOPPING
                                         && evt.GetBundle().GetBundleId() == 0)
                                     {
                                         return;
                                     }
                                 });
                             auto bundle = InstallLib(framework.GetBundleContext(), "TestBundleA");
                             bundle.Start();
                             const auto& map = bundle.GetHeaders();
                             ASSERT_TRUE(!map.empty());
                             framework.GetBundleContext().RemoveListener(std::move(token));
                             bundle.Stop();
                             bundle.Uninstall();
                         }
                     });

    frameworkshuttingdown.set_value();
    framework.Stop();
    auto evt = framework.WaitForStop(std::chrono::milliseconds::zero());
    keepLooping = false;

    // some of the framework operations can throw based on the point at which
    // framework shutdown is happening. Catch any of those and continue, those
    // exceptions are not relevant to this test.
    try
    {
        frameworkAccessThread.get();
    }
    catch (...)
    {
    }

    ASSERT_EQ(evt.GetType(), cppmicroservices::FrameworkEvent::Type::FRAMEWORK_STOPPED);
}
#endif

US_MSVC_POP_WARNING
