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


#ifndef USGETBUNDLECONTEXT_H
#define USGETBUNDLECONTEXT_H

#ifndef US_BUNDLE_NAME
#error Missing preprocessor define US_BUNDLE_NAME
#endif

#include <usBundleContext.h>
#include <usBundleContextPrivate.h>
#include <usBundleUtils.h>

#include <memory>
#include <cstring>

namespace us {

/**
 * \ingroup MicroServices
 *
 * \brief Returns the bundle context of the calling bundle.
 *
 * This function allows easy access to the BundleContext instance from
 * inside a C++ Micro Services bundle.
 *
 * \return The BundleContext of the calling bundle.
 */
static inline BundleContext GetBundleContext()
{
  BundleContext(*getBundleContext)(void) = &GetBundleContext;

  void* GetBundleContextPtr = nullptr;
  std::memcpy(&GetBundleContextPtr, &getBundleContext, sizeof(void*));
  std::string libPath(BundleUtils::GetLibraryPath(GetBundleContextPtr));

  BundleContextPrivate*(*getBundleContextInst)(void) = nullptr;
  std::string get_bundle_context_inst("_us_get_bundle_context_instance_" US_STR(US_BUNDLE_NAME));
  void* getBundleContextInstSym = BundleUtils::GetSymbol(US_STR(US_BUNDLE_NAME), libPath, get_bundle_context_inst.c_str());
  std::memcpy(&getBundleContextInst, &getBundleContextInstSym, sizeof(void*));

  if (getBundleContextInst)
  {
    BundleContextPrivate* ctx = getBundleContextInst();
    return ctx ? MakeBundleContext(ctx->shared_from_this()) : BundleContext{};
  }

  return BundleContext{};
}

}

#endif // USGETBUNDLECONTEXT_H
