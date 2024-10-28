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

#ifndef TestUtils_hpp
#define TestUtils_hpp

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceReference.h>
#include "boost/asio/async_result.hpp"
#include "boost/asio/packaged_task.hpp"
#include "boost/asio/post.hpp"
#include "boost/asio/thread_pool.hpp"
#include <cppmicroservices/asyncworkservice/AsyncWorkService.hpp>
#include <random>
#include <string>

namespace test
{

    template <typename Task, typename Predicate>
    bool
    RepeatTaskUntilOrTimeout(Task&& t, Predicate&& p)
    {
        using namespace std::chrono;
        auto startTime = system_clock::now();
        do
        {
            t();
            duration<double> duration = system_clock::now() - startTime;
            if (duration > milliseconds(30000))
            {
                return false;
            }
        } while (!p());
        return true;
    }

    /**
     * Convenience method to allow test cases to access path to bundle information.
     */
    std::unordered_map<std::string, std::string> GetPathInfo();

    /**
     * Convenience Method to install but not start a bundle given the bundle's symbolic name.
     */
    void InstallLib(::cppmicroservices::BundleContext frameworkCtx, std::string const& libName);

    /**
     * Convenience Method to install and start a bundle given the bundle's symbolic name.
     */
    cppmicroservices::Bundle InstallAndStartBundle(::cppmicroservices::BundleContext frameworkCtx,
                                                   std::string const& libName);

    /**
     * Convenience Method to install and start DS.
     */
    void InstallAndStartDS(::cppmicroservices::BundleContext frameworkCtx);

    /**
     * Convenience Method to install and start ConfigurationAdmin.
     */
    void InstallAndStartConfigAdmin(::cppmicroservices::BundleContext frameworkCtx);

    /**
     * Convenience Method to extract service-id from a service reference
     */
    long GetServiceId(::cppmicroservices::ServiceReferenceBase const& sRef);

    /**
     * Returns the file path of declarative services runtime plugin
     */
    std::string GetDSRuntimePluginFilePath();

    /**
     * Returns the file path of Configuration Admin runtime plugin
     */

    std::string GetConfigAdminRuntimePluginFilePath();

    /**
     * Returns the path to the test bundles folder
     */
    std::string GetTestPluginsPath();

    /**
     * Method to check if a bundle is loaded in current process
     */
    bool isBundleLoadedInThisProcess(std::string bundleName);

    class AsyncWorkServiceThreadPool : public cppmicroservices::async::AsyncWorkService
    {
      public:
        AsyncWorkServiceThreadPool(int nThreads);

        ~AsyncWorkServiceThreadPool() override;

        void
        post(std::packaged_task<void()>&& task) override;

      private:
        std::shared_ptr<boost::asio::thread_pool> threadpool;
    };

} // namespace test

#endif /* TestUtils_hpp */
