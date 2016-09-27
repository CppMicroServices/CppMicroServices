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

#include "cppmicroservices/FrameworkFactory.h"

#include "cppmicroservices/Framework.h"

#include "FrameworkPrivate.h"

namespace cppmicroservices {

// A helper class to shut down the framework when all external
// references (Bundle, BundleContext, etc.) are destroyed. It
// also manages the lifetime of the CoreBundleContext instance.
struct CoreBundleContextHolder
{
  CoreBundleContextHolder(std::unique_ptr<CoreBundleContext> ctx)
    : ctx(std::move(ctx))
  {}

  ~CoreBundleContextHolder()
  {
    auto const state = ctx->systemBundle->state.load();

    DIAG_LOG(*ctx->sink) << "Bundle state is  " << state << "\n";
    // The framework may have already completed its stop. In this case,
    // nothing needs to be done; the framework can be destroyed.
    // Allowing a call to WaitForStop can cause a crash during static DLL
    // destruction on Windows even if the framework was explicitly stopped
    // prior to static destruction.
    if (Bundle::STATE_INSTALLED == state || Bundle::STATE_RESOLVED == state) return;

    if (((Bundle::STATE_STARTING | Bundle::STATE_ACTIVE) & state) == 0)
    {
      // Call WaitForStop in case some did call Framework::Stop()
      // but didn't wait for it. This joins with a potentially
      // running framework shut down thread.
      FrameworkEvent fe = ctx->systemBundle->WaitForStop(std::chrono::milliseconds::zero());
      DIAG_LOG(*ctx->sink) << "dtor waiting for framework stop... " << fe << "\n";
      return;
    }

    // Create a new CoreBundleContext holder, in case some event listener
    // gets the system bundle and starts it again.
    auto fwCtx = ctx.get();
    std::shared_ptr<CoreBundleContext> holder(std::make_shared<CoreBundleContextHolder>(std::move(ctx)), fwCtx);
    holder->SetThis(holder);
    holder->systemBundle->Shutdown(false);
    FrameworkEvent fe = holder->systemBundle->WaitForStop(std::chrono::milliseconds::zero());
    DIAG_LOG(*fwCtx->sink) << "dtor fell through to Shutdown/WaitForStop... " << fe << "\n";
  }

  std::unique_ptr<CoreBundleContext> ctx;
};

Framework FrameworkFactory::NewFramework(const std::map<std::string, Any>& configuration, std::ostream* logger)
{
  std::unique_ptr<CoreBundleContext> ctx(new CoreBundleContext(configuration, logger));
  auto fwCtx = ctx.get();
  std::shared_ptr<CoreBundleContext> holder(std::make_shared<CoreBundleContextHolder>(std::move(ctx)), fwCtx);
  holder->SetThis(holder);
  return Framework(holder->systemBundle);
}

}
