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
#include "usTestUtilModuleListener.h"
#include "usTestingMacros.h"
#include "usTestingConfig.h"

US_USE_NAMESPACE

namespace
{
    void TestDefaultConfig()
    {
        FrameworkFactory factory;

        Framework* f = factory.NewFramework(std::map<std::string, std::string>());
        US_TEST_CONDITION(f, "Test Framework instantiation")

        // TODO: When a reasonable default configuration is determined, test for it here...

        delete f;
    }

    void TestCustomConfig()
    {
        std::map < std::string, std::string > configuration;
        configuration.insert(std::pair<std::string, std::string>("org.osgi.framework.security", "osgi"));
        configuration.insert(std::pair<std::string, std::string>("org.osgi.framework.startlevel.beginning", "0"));
        configuration.insert(std::pair<std::string, std::string>("org.osgi.framework.bsnversion", "single"));
        configuration.insert(std::pair<std::string, std::string>("org.osgi.framework.custom1", "foo"));
        configuration.insert(std::pair<std::string, std::string>("org.osgi.framework.custom2", "bar"));

        FrameworkFactory factory;

        Framework* f = factory.NewFramework(configuration);
        US_TEST_CONDITION(f, "Test Framework instantiation with custom configuration")

        f->Start();
        US_TEST_CONDITION("osgi" == f->GetProperty("org.osgi.framework.security").ToString(), "Test Framework custom launch properties")
        US_TEST_CONDITION("0" == f->GetProperty("org.osgi.framework.startlevel.beginning").ToString(), "Test Framework custom launch properties")
        US_TEST_CONDITION("single" == f->GetProperty("org.osgi.framework.bsnversion").ToString(), "Test Framework custom launch properties")
        US_TEST_CONDITION("foo" == f->GetProperty("org.osgi.framework.custom1").ToString(), "Test Framework custom launch properties")
        US_TEST_CONDITION("bar" == f->GetProperty("org.osgi.framework.custom2").ToString(), "Test Framework custom launch properties")

        delete f;
    }

    void TestProperties()
    {
        FrameworkFactory factory;

        Framework* f = factory.NewFramework(std::map<std::string, std::string>());
        f->Start();
        US_TEST_CONDITION(f->GetLocation() == "System Bundle", "Test Framework Bundle Location");
        US_TEST_CONDITION(f->GetName() == US_CORE_FRAMEWORK_NAME, "Test Framework Bundle Name");
        // TODO: fix once the system bundle id is set to 0
        US_TEST_CONDITION(f->GetModuleId() == 1, "Test Framework Bundle Id");

        delete f;
    }

    void TestLifeCycle()
    {
        TestModuleListener listener;
        FrameworkFactory factory;
        std::vector<ModuleEvent> pEvts;

        Framework* f = factory.NewFramework(std::map<std::string, std::string>());
        f->Start();
        
        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")

        US_TEST_CONDITION(f->IsLoaded(), "Check framework is in the Start state")

        f->GetModuleContext()->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);

        f->Stop();
        US_TEST_CONDITION(!f->IsLoaded(), "Check framework is in the Stop state")

        pEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, f));

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check framework bundle event listener")

        // Test that uninstalling a framework throws an exception
        try
        {
            f->Uninstall();
            US_TEST_FAILED_MSG(<< "Failed to throw uninstall exception")
        }
        catch (const std::runtime_error&)
        {

        }
        catch (...)
        {
            US_TEST_FAILED_MSG(<< "Failed to throw correct exception type")
        }

        // Test that all bundles in the Start state are stopped when the framework is stopped.
        f->Start();
        InstallTestBundle(f->GetModuleContext(), "TestModuleA");

        Module* moduleA = f->GetModuleContext()->GetModule("TestModuleA");
        moduleA->Start();

        // Stopping the framework stops all active bundles.
        f->Stop();

        US_TEST_CONDITION(!moduleA->IsLoaded(), "Check that TestModuleA is in the Stop state")
        US_TEST_CONDITION(!f->IsLoaded(), "Check framework is in the Stop state")

        delete f;

    }

    void TestEvents()
    {
        TestModuleListener listener;
        std::vector<ModuleEvent> pEvts;
        std::vector<ModuleEvent> pStopEvts;
        FrameworkFactory factory;

        Framework* f = factory.NewFramework(std::map<std::string, std::string>());

        f->Start();

        ModuleContext* fmc = f->GetModuleContext();
        fmc->AddModuleListener(&listener, &TestModuleListener::ModuleChanged);

        // The bundles used to test bundle events when stopping the framework
        Module* module = InstallTestBundle(fmc, "TestModuleA");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleA2");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleB");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleH");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleM");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleR");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleRA");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleRL");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleS");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleSL1");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleSL3");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));
        module = InstallTestBundle(fmc, "TestModuleSL4");
        pEvts.push_back(ModuleEvent(ModuleEvent::INSTALLED, module));

        std::vector<Module*> modules(fmc->GetModules());
        for (std::vector<Module*>::iterator iter = modules.begin();
            iter != modules.end(); ++iter)
        {
            (*iter)->Start();
            // no events will be fired for the framework, its already active at this point
            if ((*iter) != f)
            {
                pEvts.push_back(ModuleEvent(ModuleEvent::LOADING, (*iter)));
                pEvts.push_back(ModuleEvent(ModuleEvent::LOADED, (*iter)));

                // bundles will be stopped in the same order in which they were started.
                // It is easier to maintain this test if the stop events are setup in the 
                // right order here, while the bundles are starting, instead of hard coding 
                // the order of events somewhere else.
                // Doing it this way also tests the order in which starting and stopping 
                // bundles occurs and when their events are fired.
                pStopEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, (*iter)));
                pStopEvts.push_back(ModuleEvent(ModuleEvent::UNLOADED, (*iter)));
            }
        }

        US_TEST_CONDITION(listener.CheckListenerEvents(pEvts), "Check for bundle start events")

        // Remember, the framework is stopped last, after all bundles are stopped.
        pStopEvts.push_back(ModuleEvent(ModuleEvent::UNLOADING, f));

        // Stopping the framework stops all active bundles.
        f->Stop();
        
        US_TEST_CONDITION(listener.CheckListenerEvents(pStopEvts), "Check for bundle stop events")

        delete f;
    }
}

int usFrameworkTest(int /*argc*/, char* /*argv*/[])
{
    US_TEST_BEGIN("FrameworkTest");

    TestDefaultConfig();
    TestCustomConfig();
    TestProperties();
    TestLifeCycle();
    TestEvents();

    US_TEST_END()
}