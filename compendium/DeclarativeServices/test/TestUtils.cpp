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

#include "TestUtils.hpp"
#include "DSTestingConfig.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/Constants.h"
#include "cppmicroservices/util/Error.h"
#include "cppmicroservices/util/FileSystem.h"
#include <iostream>

#if defined(US_PLATFORM_WINDOWS)
#    include <Windows.h>
#    include <psapi.h>
#else
#    include <fstream>
#    include <unistd.h>
#endif

#if defined(US_PLATFORM_LINUX)
#    include <linux/limits.h>
#endif

namespace
{

    std::string
    PathToLib(std::string const& libName)
    {
        return (cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP + US_LIB_PREFIX + libName
                + US_LIB_POSTFIX + US_LIB_EXT);
    }

} // namespace

namespace test
{

    std::unordered_map<std::string, std::string>
    GetPathInfo()
    {
        std::unordered_map<std::string, std::string> pathInfo = {
            { "libPath", cppmicroservices::testing::LIB_PATH },
            { "dirSep", std::string(1, cppmicroservices::util::DIR_SEP) },
            { "usLibPrefix", US_LIB_PREFIX },
            { "usLibPostfix", US_LIB_POSTFIX },
            { "usLibExt", US_LIB_EXT }
        };
        return pathInfo;
    }

    void
    InstallLib(
#if defined(US_BUILD_SHARED_LIBS)
        ::cppmicroservices::BundleContext frameworkCtx,
        std::string const& libName
#else
        ::cppmicroservices::BundleContext,
        std::string const&
#endif
    )
    {
#if defined(US_BUILD_SHARED_LIBS)
        frameworkCtx.InstallBundles(PathToLib(libName));
#endif
    }

    cppmicroservices::Bundle
    InstallAndStartBundle(::cppmicroservices::BundleContext frameworkCtx, std::string const& libName)
    {
        std::vector<cppmicroservices::Bundle> bundles;

#if defined(US_BUILD_SHARED_LIBS)
        bundles = frameworkCtx.InstallBundles(cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP
                                              + US_LIB_PREFIX + libName + US_LIB_POSTFIX + US_LIB_EXT);
#else
        bundles = frameworkCtx.GetBundles();
#endif

        for (auto b : bundles)
        {
            if (b.GetSymbolicName() == libName)
            {
                b.Start();
                return b;
            }
        }
        return {};
    }

    long
    GetServiceId(::cppmicroservices::ServiceReferenceBase const& sRef)
    {
        long serviceId = 0;
        try
        {
            if (sRef)
            {
                serviceId
                    = cppmicroservices::any_cast<long int>(sRef.GetProperty(::cppmicroservices::Constants::SERVICE_ID));
            }
        }
        catch (std::exception const& e)
        {
            std::cout << "Exception: " << e.what() << std::endl;
            throw;
        }
        return serviceId;
    }

    std::string
    GetDSRuntimePluginFilePath()
    {
        std::string libName { "DeclarativeServices" };
#if defined(US_PLATFORM_WINDOWS)
        libName += US_DeclarativeServices_VERSION_MAJOR;
#endif
        return PathToLib(libName);
    }

    std::string
    GetTestPluginsPath()
    {
        return (cppmicroservices::testing::LIB_PATH + cppmicroservices::util::DIR_SEP);
    }

    void
    InstallAndStartDS(::cppmicroservices::BundleContext frameworkCtx)
    {
        std::vector<cppmicroservices::Bundle> bundles;
        auto dsPluginPath = test::GetDSRuntimePluginFilePath();

#if defined(US_BUILD_SHARED_LIBS)
        bundles = frameworkCtx.InstallBundles(dsPluginPath);
#else
        bundles = frameworkCtx.GetBundles();
#endif

        for (auto b : bundles)
        {
            b.Start();
        }
    }

    std::string
    GetConfigAdminRuntimePluginFilePath()
    {
        std::string libName { "ConfigurationAdmin" };
#if defined(US_PLATFORM_WINDOWS)
        // libName += US_ConfigurationAdmin_VERSION_MAJOR;
        //  This is a hack for the time being.
        //  TODO: revisit changing the hard-coded "1" to the ConfigAdmin version dynamically
        libName += "1";
#endif
        return PathToLib(libName);
    }

    void
    InstallAndStartConfigAdmin(::cppmicroservices::BundleContext frameworkCtx)
    {
        std::vector<cppmicroservices::Bundle> bundles;
        auto cmPluginPath = test::GetConfigAdminRuntimePluginFilePath();

#if defined(US_BUILD_SHARED_LIBS)
        bundles = frameworkCtx.InstallBundles(cmPluginPath);
#else
        bundles = frameworkCtx.GetBundles();
#endif

        for (auto b : bundles)
        {
            b.Start();
        }
    }

    bool
    isBundleLoadedInThisProcess(std::string bundleName)
    {
#if defined(US_PLATFORM_WINDOWS)

        HMODULE hMods[1024];
        DWORD cbNeeded;

        HANDLE hProcess = GetCurrentProcess();

        if (!EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
        {
            std::cout << "FAILURE:\n"
                      << "EnumProcessModules failed : " << cppmicroservices::util::GetLastWin32ErrorStr() << std::endl;
            SetLastError(0);
            return false;
        }

        if ((sizeof(hMods) < cbNeeded))
        {
            std::cout << "WARNING:\n"
                      << "EnumProcessModules : Size of array is too small to hold all "
                         "module handles"
                      << std::endl;
        }

        TCHAR szModName[MAX_PATH * 2];
        std::size_t found;

        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            if (!GetModuleFileNameA(hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                std::cout << "WARNING:\n"
                          << "GetModuleFileNameA failed :" << cppmicroservices::util::GetLastWin32ErrorStr()
                          << std::endl;
                SetLastError(0);
            }

            found = std::string(szModName).find(bundleName);
            if (found != std::string::npos)
            {
                return true;
            }
        }

        return false;
#else
        auto pid_t = getpid();
        std::string command("lsof -p " + std::to_string(pid_t));
        FILE* fd = popen(command.c_str(), "r");
        if (nullptr == fd)
        {
            std::cout << "FAILURE:\n"
                      << "popen failed" << std::endl;
            return false;
        }

        std::size_t found;
        char buf[PATH_MAX];
        while (nullptr != fgets(buf, PATH_MAX, fd))
        {
            found = std::string(buf).find(bundleName);
            if (found != std::string::npos)
            {
                if (-1 == pclose(fd))
                {
                    std::cout << "WARNING:\n"
                              << "pclose failed" << std::endl;
                }
                return true;
            }
        }

        if (-1 == pclose(fd))
        {
            std::cout << "WARNING:\n"
                      << "pclose failed" << std::endl;
        }
        return false;
#endif
    }

    AsyncWorkServiceThreadPool::AsyncWorkServiceThreadPool(int nThreads) : cppmicroservices::async::AsyncWorkService(), threadpool(std::make_shared<boost::asio::thread_pool>(nThreads))
    {
    }

    AsyncWorkServiceThreadPool::~AsyncWorkServiceThreadPool()
    {
        try
        {
            if (threadpool)
            {
                try
                {
                    threadpool->join();
                }
                catch (...)
                {
                    //
                }
            }
        }
        catch (...)
        {
            //
        }
    }

    void
    AsyncWorkServiceThreadPool::post(std::packaged_task<void()>&& task)
    {
        using Sig = void();
        using Result = boost::asio::async_result<decltype(task), Sig>;
        using Handler = typename Result::completion_handler_type;

        Handler handler{std::move(task)};
        Result result(handler);

        boost::asio::post(threadpool->get_executor(), [handler = std::move(handler)]() mutable { handler(); });
    }

} // namespace test
