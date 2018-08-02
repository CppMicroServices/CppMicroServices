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

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/BundleContext.h"
#include "gtest/gtest.h"
#include <vector>
#include <future>
#include <thread>

using namespace cppmicroservices;

namespace test {
  class ITestService {
  public:
    virtual ~ITestService() {};
    virtual int getValue() const = 0;
  };

  struct TestServiceA : public ITestService
  {
    int getValue() const
    {
      return 0;
    }
  };
}

TEST(MakeInterfaceMapTest, ConcurrencyTest)
{
  FrameworkFactory factory;
  auto framework = factory.NewFramework();
  framework.Init();
  BundleContext fCtx = framework.GetBundleContext();

  const auto numparallel = std::thread::hardware_concurrency();

  {
    std::vector<std::future<void>> futures;
    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    std::vector<std::promise<void>> threadReadies(numparallel);
    std::vector<std::future<void>> threadReadyFuts;
    for(auto& p : threadReadies)
    {
      threadReadyFuts.push_back(p.get_future());
    }
    auto addListener = [ready] (std::promise<void> prom)
    {
      prom.set_value();
      ready.wait();
      for (int k = 0; k < 500000; ++k)
      {
        InterfaceMapConstPtr im = MakeInterfaceMap<test::ITestService>(std::make_shared<test::TestServiceA>());
        (void)im;
      }
    };

    try
    {
      for (unsigned int i = 0; i < numparallel; i++)
      {
        futures.push_back(std::async(std::launch::async,
                                     addListener, std::move(threadReadies[i])));
      }
      for (auto& threadReady : threadReadyFuts)
      {
        threadReady.wait();
      }

      go.set_value();

      for (auto& future: futures)
      {
        future.get();
      }
    }
    catch (...)
    {
      go.set_value();
      throw;
    }
  }
}
