/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#include "usTestingMacros.h"

#include <usGetModuleContext.h>
#include <usModuleContext.h>

US_USE_NAMESPACE

#ifdef US_PLATFORM_APPLE
#include <mach/mach_time.h>
#elif defined(US_PLATFORM_POSIX)
#include <time.h>
#include <unistd.h>
#ifndef _POSIX_MONOTONIC_CLOCK
#error Monotonic clock support missing on this POSIX platform
#endif
#elif defined(US_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRA_LEAN
#define VC_EXTRA_LEAN
#endif
#include <windows.h>
#else
#error High precision timer support nod available on this platform
#endif

#include <vector>

class HighPrecisionTimer
{

public:

  inline HighPrecisionTimer();

  inline void Start();

  inline long long ElapsedMilli();

  inline long long ElapsedMicro();

private:

#ifdef US_PLATFORM_APPLE
  static double timeConvert;
  uint64_t startTime;
#elif defined(US_PLATFORM_POSIX)
  timespec startTime;
#elif defined(US_PLATFORM_WINDOWS)
  LARGE_INTEGER timerFrequency;
  LARGE_INTEGER startTime;
#endif
};

#ifdef US_PLATFORM_APPLE

double HighPrecisionTimer::timeConvert = 0.0;

inline HighPrecisionTimer::HighPrecisionTimer()
: startTime(0)
{
  if (timeConvert == 0)
  {
    mach_timebase_info_data_t timeBase;
    mach_timebase_info(&timeBase);
    timeConvert = static_cast<double>(timeBase.numer) / static_cast<double>(timeBase.denom) / 1000.0;
  }
}

inline void HighPrecisionTimer::Start()
{
  startTime = mach_absolute_time();
}

inline long long HighPrecisionTimer::ElapsedMilli()
{
  uint64_t current = mach_absolute_time();
  return static_cast<double>(current - startTime) * timeConvert / 1000.0;
}

inline long long HighPrecisionTimer::ElapsedMicro()
{
  uint64_t current = mach_absolute_time();
  return static_cast<double>(current - startTime) * timeConvert;
}

#elif defined(US_PLATFORM_POSIX)

inline HighPrecisionTimer::HighPrecisionTimer()
{
  startTime.tv_nsec = 0;
  startTime.tv_sec = 0;
}

inline void HighPrecisionTimer::Start()
{
  clock_gettime(CLOCK_MONOTONIC, &startTime);
}

inline long long HighPrecisionTimer::ElapsedMilli()
{
  timespec current;
  clock_gettime(CLOCK_MONOTONIC, &current);
  return (static_cast<long long>(current.tv_sec)*1000 + current.tv_nsec/1000/1000) -
      (static_cast<long long>(startTime.tv_sec)*1000 + startTime.tv_nsec/1000/1000);
}

inline long long HighPrecisionTimer::ElapsedMicro()
{
  timespec current;
  clock_gettime(CLOCK_MONOTONIC, &current);
  return (static_cast<long long>(current.tv_sec)*1000*1000 + current.tv_nsec/1000) -
      (static_cast<long long>(startTime.tv_sec)*1000*1000 + startTime.tv_nsec/1000);
}

#elif defined(US_PLATFORM_WINDOWS)

inline HighPrecisionTimer::HighPrecisionTimer()
{
  if (!QueryPerformanceFrequency(&timerFrequency))
    throw std::runtime_error("QueryPerformanceFrequency() failed");
}

inline void HighPrecisionTimer::Start()
{
  //DWORD_PTR oldmask = SetThreadAffinityMask(GetCurrentThread(), 0);
  QueryPerformanceCounter(&startTime);
  //SetThreadAffinityMask(GetCurrentThread(), oldmask);
}

inline long long HighPrecisionTimer::ElapsedMilli()
{
  LARGE_INTEGER current;
  QueryPerformanceCounter(&current);
  return (current.QuadPart - startTime.QuadPart) / (timerFrequency.QuadPart / 1000);
}

inline long long HighPrecisionTimer::ElapsedMicro()
{
  LARGE_INTEGER current;
  QueryPerformanceCounter(&current);
  return (current.QuadPart - startTime.QuadPart) / (timerFrequency.QuadPart / (1000*1000));
}

#endif

class MyServiceListener;

struct IPerfTestService
{
  virtual ~IPerfTestService() {}
};


class ServiceRegistryPerformanceTest
{

private:

  friend class MyServiceListener;

  ModuleContext* mc;

  int nListeners;
  int nServices;

  std::size_t nRegistered;
  std::size_t nUnregistering;
  std::size_t nModified;

  std::vector<ServiceRegistration<IPerfTestService> > regs;
  std::vector<MyServiceListener*> listeners;
  std::vector<IPerfTestService*> services;

public:

  ServiceRegistryPerformanceTest(ModuleContext* context);

  void InitTestCase();
  void CleanupTestCase();

  void TestAddListeners();
  void TestRegisterServices();

  void TestModifyServices();
  void TestUnregisterServices();

private:

  std::ostream& Log() const
  {
    return std::cout;
  }

  void AddListeners(int n);
  void RegisterServices(int n);
  void ModifyServices();
  void UnregisterServices();

};

class MyServiceListener
{

private:

  ServiceRegistryPerformanceTest* ts;

public:

  MyServiceListener(ServiceRegistryPerformanceTest* ts)
    : ts(ts)
  {
  }

  void ServiceChanged(const ServiceEvent ev)
  {
    switch(ev.GetType())
    {
    case ServiceEvent::REGISTERED:
      ts->nRegistered++;
      break;
    case ServiceEvent::UNREGISTERING:
      ts->nUnregistering++;
      break;
    case ServiceEvent::MODIFIED:
      ts->nModified++;
      break;
    default:
      break;
    }
  }
};


ServiceRegistryPerformanceTest::ServiceRegistryPerformanceTest(ModuleContext* context)
  : mc(context)
  , nListeners(100)
  , nServices(1000)
  , nRegistered(0)
  , nUnregistering(0)
  , nModified(0)
{
}

void ServiceRegistryPerformanceTest::InitTestCase()
{
  Log() << "Initialize event counters\n";

  nRegistered    = 0;
  nUnregistering = 0;
  nModified      = 0;
}

void ServiceRegistryPerformanceTest::CleanupTestCase()
{
  Log() << "Remove all service listeners\n";

  for(std::size_t i = 0; i < listeners.size(); i++)
  {
    try
    {
      MyServiceListener* l = listeners[i];
      mc->RemoveServiceListener(l, &MyServiceListener::ServiceChanged);
      delete l;
    }
    catch (const std::exception& e)
    {
      Log() << e.what();
    }
  }
  listeners.clear();
}

void ServiceRegistryPerformanceTest::TestAddListeners()
{
  AddListeners(nListeners);
}

void ServiceRegistryPerformanceTest::AddListeners(int n)
{
  Log() << "adding " << n << " service listeners\n";
  for(int i = 0; i < n; i++)
  {
    MyServiceListener* l = new MyServiceListener(this);
    try
    {
      listeners.push_back(l);
      mc->AddServiceListener(l, &MyServiceListener::ServiceChanged, "(perf.service.value>=0)");
    }
    catch (const std::exception& e)
    {
      Log() << e.what();
    }
  }
  Log() << "listener count=" << listeners.size() << "\n";
}

void ServiceRegistryPerformanceTest::TestRegisterServices()
{
  Log() << "Register services, and check that we get #of services ("
        << nServices << ") * #of listeners (" << nListeners << ")  REGISTERED events\n";

  Log() << "registering " << nServices << " services, listener count=" << listeners.size() << "\n";

  HighPrecisionTimer t;
  t.Start();
  RegisterServices(nServices);
  long long ms = t.ElapsedMilli();
  Log() << "register took " << ms << "ms\n";
  US_TEST_CONDITION_REQUIRED(nServices * listeners.size() == nRegistered,
                             "# REGISTERED events must be same as # of registered services  * # of listeners");
}

void ServiceRegistryPerformanceTest::RegisterServices(int n)
{
  class PerfTestService : public IPerfTestService
  {
  };

  std::string pid("my.service.");

  for(int i = 0; i < n; i++)
  {
    ServiceProperties props;
    std::stringstream ss;
    ss << pid << i;
    props["service.pid"] = ss.str();
    props["perf.service.value"] = i+1;

    PerfTestService* service = new PerfTestService();
    services.push_back(service);
    ServiceRegistration<IPerfTestService> reg =
        mc->RegisterService<IPerfTestService>(service, props);
    regs.push_back(reg);
  }
}

void ServiceRegistryPerformanceTest::TestModifyServices()
{
  Log() << "Modify all services, and check that we get #of services ("
        << nServices << ") * #of listeners (" << nListeners << ")  MODIFIED events\n";

  HighPrecisionTimer t;
  t.Start();
  ModifyServices();
  long long ms = t.ElapsedMilli();
  Log() << "modify took " << ms << "ms\n";
  US_TEST_CONDITION_REQUIRED(nServices * listeners.size() == nModified,
                             "# MODIFIED events must be same as # of modified services  * # of listeners");
}

void ServiceRegistryPerformanceTest::ModifyServices()
{
  Log() << "modifying " << regs.size() << " services, listener count=" << listeners.size() << "\n";

  for(std::size_t i = 0; i < regs.size(); i++)
  {
    ServiceRegistration<IPerfTestService> reg = regs[i];
    ServiceProperties props;
    props["perf.service.value"] = i * 2;
    reg.SetProperties(props);
  }
}

void ServiceRegistryPerformanceTest::TestUnregisterServices()
{
  Log() << "Unregister all services, and check that we get #of services ("
        << nServices << ") * #of listeners (" << nListeners
        << ")  UNREGISTERING events\n";

  HighPrecisionTimer t;
  t.Start();
  UnregisterServices();
  long long ms = t.ElapsedMilli();
  Log() <<  "unregister took " << ms << "ms\n";
  US_TEST_CONDITION_REQUIRED(nServices * listeners.size() == nUnregistering, "# UNREGISTERING events must be same as # of (un)registered services * # of listeners");
}

void ServiceRegistryPerformanceTest::UnregisterServices()
{
  Log() << "unregistering " << regs.size() << " services, listener count="
        << listeners.size() << "\n";
  for(std::size_t i = 0; i < regs.size(); i++)
  {
    ServiceRegistration<IPerfTestService> reg = regs[i];
    reg.Unregister();
  }
  regs.clear();
}


int usServiceRegistryPerformanceTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ServiceRegistryPerformanceTest")

  ServiceRegistryPerformanceTest perfTest(GetModuleContext());
  perfTest.InitTestCase();
  perfTest.TestAddListeners();
  perfTest.TestRegisterServices();
  perfTest.TestModifyServices();
  perfTest.TestUnregisterServices();
  perfTest.CleanupTestCase();

  US_TEST_END()
}
