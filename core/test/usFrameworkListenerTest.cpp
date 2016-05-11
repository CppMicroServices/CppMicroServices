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

#include <usFrameworkFactory.h>
#include <usFramework.h>
#include <usFrameworkEvent.h>
#include <usBundleContext.h>

#include <usTestUtils.h>
#include <usTestingMacros.h>
#include <usTestingConfig.h>

#include <thread>
#include <vector>

using namespace us;

#ifdef ERROR
#undef ERROR
#endif

namespace {

class TestFrameworkListener
{
public:
  TestFrameworkListener() : _events() {}
  virtual ~TestFrameworkListener() {};

  std::size_t events_received() const { return _events.size(); }

  bool CheckEvents(const std::vector<FrameworkEvent>& events)
  {
	bool listenState = true; // assume success
	if (events.size() != _events.size())
	{
      listenState = false;
      US_TEST_OUTPUT( << "*** Framework event mismatch ***\n expected "
                        << events.size() << " event(s)\n found "
                        << _events.size() << " event(s).");

      const std::size_t max = events.size() > _events.size() ? events.size() : _events.size();
      for (std::size_t i = 0; i < max; ++i)
      {
        const FrameworkEvent& pE = i < events.size() ? events[i] : FrameworkEvent();
        const FrameworkEvent& pR = i < _events.size() ? _events[i] : FrameworkEvent();
        US_TEST_OUTPUT( << " - " << pE << " - " << pR);
      }
	}
	else
	{
	  for (std::size_t i = 0; i < events.size(); ++i)
	  {
		const FrameworkEvent& pE = events[i];
		const FrameworkEvent& pR = _events[i];
		if (pE.GetType() != pR.GetType() || pE.GetBundle() != pR.GetBundle())
		{
          listenState = false;
		  US_TEST_OUTPUT( << "*** Wrong framework event ***\n found " << pR << "\n expected " << pE);
		}
	  }
	}

	_events.clear();
	return listenState;
  }

  void frameworkEvent(const FrameworkEvent& evt)
  {
    _events.push_back(evt);
    std::cout << evt << std::endl;
  }

private:
  std::vector<FrameworkEvent> _events;
};

void testStartStopFrameworkEvents() 
{
  auto f = FrameworkFactory().NewFramework();
  f->Start();

  // @todo once Framework::init() is implemented, add the listener after the init()
  // and before the Start().
  TestFrameworkListener l;
  f->GetBundleContext()->AddFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);
  f->Stop();

  std::vector<FrameworkEvent> events;
  events.push_back(FrameworkEvent(FrameworkEvent::Type::STARTING, f->shared_from_this(), "Framework Starting"));
  events.push_back(FrameworkEvent(FrameworkEvent::Type::STARTED, f->shared_from_this(), "Framework Started"));
  events.push_back(FrameworkEvent(FrameworkEvent::Type::STOPPING, f->shared_from_this(), "Framework Stopping"));
  events.push_back(FrameworkEvent(FrameworkEvent::Type::STOPPED, f->shared_from_this(), "Framework Stopped"));
  US_TEST_CONDITION_REQUIRED(l.CheckEvents(events), "Test for the correct number and order of Framework start/stop events.");
}

void testAddRemoveFrameworkListener() 
{
  auto f = FrameworkFactory().NewFramework();
  f->Start();

  // Test that the lambda is removed correctly if the lambda is referenced in a variable
  auto listener = [](const FrameworkEvent&) { US_TEST_FAILED_MSG(<< "Failed to remove framework listener"); };
  f->GetBundleContext()->AddFrameworkListener(listener);
  f->GetBundleContext()->RemoveFrameworkListener(listener);

  // test listener removal...
  TestFrameworkListener l;
  f->GetBundleContext()->AddFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);
  f->GetBundleContext()->RemoveFrameworkListener(&l, &TestFrameworkListener::frameworkEvent);

  f->Start();   // generate framework events
  US_TEST_CONDITION_REQUIRED(l.CheckEvents(std::vector<FrameworkEvent>()), "Test removal of listener");

  int count1(0);
  int count2(0);
  auto listener_callback_counter1 = [&count1](const FrameworkEvent&) { ++count1; std::cout << "listener_callback_counter1: call count " << count1 << std::endl; };
  auto listener_callback_counter2 = [&count2](const FrameworkEvent&) { ++count2; std::cout << "listener_callback_counter2: call count " << count2 << std::endl; };
  auto listener_callback_throw = [](const FrameworkEvent&) { throw std::runtime_error("boo"); };
  // @fixme issue #95 ... can't add more than one lambda defined listener
  f->GetBundleContext()->AddFrameworkListener(listener_callback_counter1);
  f->GetBundleContext()->AddFrameworkListener(listener_callback_counter2);
  f->GetBundleContext()->AddFrameworkListener(listener_callback_throw);

  // generate 2 framework events (starting, started)
  f->Start();
  US_TEST_CONDITION_REQUIRED(count1 == 2, "Test that multiple framework listeners were called");
  US_TEST_CONDITION_REQUIRED(count2 == 2, "Test that multiple framework listeners were called");

  f->GetBundleContext()->RemoveFrameworkListener(listener_callback_counter1);
  f->GetBundleContext()->RemoveFrameworkListener(listener_callback_counter2);
  f->GetBundleContext()->RemoveFrameworkListener(listener_callback_throw);
  // generate 2 framework events (starting, started)
  f->Start();
  US_TEST_CONDITION_REQUIRED(count1 == 2, "Test that multiple framework listeners were NOT called after removal");
  US_TEST_CONDITION_REQUIRED(count2 == 2, "Test that multiple framework listeners were NOT called after removal");
}

void testFrameworkListenersAfterFrameworkStop() 
{
  auto f = FrameworkFactory().NewFramework();
  f->Start();
  // OSGi section 10.2.2.13 (Framework::stop API) says that event handling must be "disabled".
  // It does not say that existing framework listeners MUST be removed.
  // @assumption Framework listeners are still registered across Framework start/stop.
  int events(0);
  auto listener = [&events](const FrameworkEvent& evt) { ++events; std::cout << evt << std::endl; };
  f->GetBundleContext()->AddFrameworkListener(listener);
  f->Stop();
  f->Start();
  US_TEST_CONDITION_REQUIRED(events == 4 , "Test that the existing listener was used again on Framework Start");
}

void testFrameworkListenerThrowingInvariant() 
{
  /*
    The Framework must publish a FrameworkEvent.ERROR if a callback to an event listener generates an exception - except
    when the callback happens while delivering a FrameworkEvent.ERROR (to prevent an infinite loop).

    Tests:
    1. Given a bundle listener which throws -> verfiy a FrameworkEvent ERROR is received with the correct event info.
    2. Given a service listener which throws -> verfiy a FrameworkEvent ERROR is received with the correct event info.
    3. Given a framework listener which throws -> No FrameworkEvent is received, instead an internal log message is sent.
  */
  std::stringstream logstream;
  std::ostream sink(logstream.rdbuf());
  // Use a redirected logger to verify that the framework listener logged an
  // error message when it encountered a FrameworkEvent::ERROR coming from
  // a framework listener.
  std::map<std::string, us::Any> config{ {Framework::PROP_LOG, true} };
  auto f = FrameworkFactory().NewFramework(config, &sink);
  f->Start();
  
  bool fwk_error_received(false);
  std::string exception_string("bad callback");
  auto listener = [&fwk_error_received, &exception_string](const FrameworkEvent& evt)
    {
      try
      {
        if(evt.GetThrowable()) std::rethrow_exception(evt.GetThrowable());
      }
      catch (const std::exception& e)
      {
        if (FrameworkEvent::Type::ERROR == evt.GetType() &&
            e.what() == exception_string &&
            typeid(e).name() == typeid(std::runtime_error).name())
        {
          fwk_error_received = true;
        }
      }
    };
 
  f->GetBundleContext()->AddFrameworkListener(listener);

  // test bundle event listener
  f->GetBundleContext()->AddBundleListener([](const BundleEvent&) { throw std::runtime_error("bad callback"); });
  f->Stop();
  US_TEST_CONDITION_REQUIRED(fwk_error_received, "Test that a Framework ERROR event was received from a throwing bundle listener");
  
  f->Start();
  // @todo fix this. framework listeners should persist across framework start/stop (OSGi section 10.2.2.13)
  f->GetBundleContext()->AddFrameworkListener(listener);

  // test service event listener
  fwk_error_received = false;
  exception_string = "you sunk my battleship";
  f->GetBundleContext()->AddServiceListener([](const ServiceEvent&) { throw std::runtime_error("you sunk my battleship");  });
  auto bundleA = InstallTestBundle(f->GetBundleContext(), "TestBundleA");
  bundleA->Start();

  US_TEST_CONDITION_REQUIRED(fwk_error_received, "Test that a Framework ERROR event was received from a throwing service listener");

  // test framework event listener
  fwk_error_received = false;
  exception_string = "whoopsie!";
  // @fixme issue #95. can't add another framework listener as a lambda
  f->GetBundleContext()->AddFrameworkListener([](const FrameworkEvent&) { throw std::runtime_error("whoopsie!"); });
  // Minimally, an infinite loop will appear here if there is a problem.
  f->Stop();
  US_TEST_CONDITION_REQUIRED(false == fwk_error_received, "Test that a Framework ERROR event was NOT received from a throwing framework listener");
  US_TEST_CONDITION_REQUIRED(std::string::npos != logstream.str().find("A Framework Listener threw an exception:"), "Test for internal log message from Framework event handler");
  // @todo this will cause a crash due to the framework bundle context being destroyed on Stop().
  //  fix this once Framework::init() is implemented. 
  //  framework listeners should persist across framework start/stop (OSGi section 10.2.2.13)
  //f->GetBundleContext()->RemoveFrameworkListener(listener);
}

void testDeadLock()
{
  // test for deadlocks during Framework API re-entry from a Framework Listener callback
  auto f = FrameworkFactory().NewFramework();
  f->Start();

  auto listener = [&f](const FrameworkEvent& evt)
  {
    if (FrameworkEvent::Type::ERROR == evt.GetType())
    {
        // generate a framework event on another thread,
        // which will cause a deadlock if any mutexes are locked.
        // Doing this on the same thread would either produce
        // undefined behavior (typically a deadlock or an exception)
        std::thread t([&f]() {f->Start(); });
        t.join();
    }
  };

  f->GetBundleContext()->AddBundleListener([](const BundleEvent&) { throw std::runtime_error("bad bundle"); });
  f->GetBundleContext()->AddFrameworkListener(listener);
  auto bundleA = InstallTestBundle(f->GetBundleContext(), "TestBundleA"); // trigger the bundle listener to be called

  f->Stop();
}

} // end anonymous namespace

int usFrameworkListenerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("FrameworkListenerTest");

  auto lambda1 = [](const FrameworkEvent&) {};
  auto lambda2 = [](const FrameworkEvent&) {};
  US_TEST_CONDITION((typeid(lambda1) != typeid(lambda2)), "Test lambda type info (in)equality");
  US_TEST_CONDITION(
      std::function<void(const FrameworkEvent&)>(lambda1).target<void(const FrameworkEvent&)>() == std::function<void(const FrameworkEvent&)>(lambda2).target<void(const FrameworkEvent&)>(), 
      "Test std::function target equality"
  );

  /*
    @note Framework events will be sent like bundle and service events; synchronously.
          Framework events SHOULD be delivered asynchronously (see OSGi spec), however that's not yet supported.
    @todo test asynchronous event delivery once its supported.
  */

  testStartStopFrameworkEvents();
  testAddRemoveFrameworkListener();
  testFrameworkListenersAfterFrameworkStop();
  testFrameworkListenerThrowingInvariant();
  testDeadLock();

  US_TEST_END()
}
