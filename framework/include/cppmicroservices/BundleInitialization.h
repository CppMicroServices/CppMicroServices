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

#ifndef CPPMICROSERVICES_BUNDLEINITIALIZATION_H_TYPES
#define CPPMICROSERVICES_BUNDLEINITIALIZATION_H_TYPES

#include "cppmicroservices/GlobalConfig.h"

#include <functional>

namespace cppmicroservices
{
    class BundleContext;
    class BundleContextPrivate;
    // THIS IS NOT A TRANSFER OF OWNERSHIP. The pointed object must be kept alive for the lifetime
    // of the bundle, and then destroyed. Avoid calling this twice with different values, as you
    // could leak memory.
    using SetBundleContextPublicHook = std::function<void(BundleContext*)>;
    using SetBundleContextPrivateHook = std::function<void(BundleContextPrivate*)>;
}

#endif // CPPMICROSERVICES_BUNDLEINITIALIZATION_H_TYPES

// Header file may be included just for the hook data type
#ifdef US_BUNDLE_NAME
#ifndef CPPMICROSERVICES_BUNDLEINITIALIZATION_H_MACRO
#define CPPMICROSERVICES_BUNDLEINITIALIZATION_H_MACRO

/**
 * \ingroup MicroServices
 * \ingroup gr_macros
 *
 * \brief Creates initialization code for a bundle.
 *
 * Each bundle which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files. Further, the bundle's
 * source files must be compiled with the \c US_BUNDLE_NAME pre-processor definition
 * set to a bundle-unique identifier.
 *
 * Calling the #CPPMICROSERVICES_INITIALIZE_BUNDLE macro will initialize the bundle for use with
 * the CppMicroServices library.
 *
 * \rststar
 * .. hint::
 *
 *    If you are using CMake, consider using the provided CMake macro
 *    :cmake:command:`usFunctionGenerateBundleInit`.
 * \endrststar
 */
#define CPPMICROSERVICES_INITIALIZE_BUNDLE                                                                      \
    struct {                                                                                                    \
        bool usingPubCtx = false;                                                                               \
        cppmicroservices::BundleContextPrivate* ctx = nullptr;                                                  \
    } US_CTX_INS(US_BUNDLE_NAME);                                                                               \
                                                                                                                \
    extern "C" cppmicroservices::BundleContextPrivate* US_GET_CTX_FUNC(US_BUNDLE_NAME)(bool* isPublic);         \
    extern "C" cppmicroservices::BundleContextPrivate* US_GET_CTX_FUNC(US_BUNDLE_NAME)(bool* isPublic)          \
    {                                                                                                           \
        *isPublic = US_CTX_INS(US_BUNDLE_NAME).usingPubCtx;                                                     \
        return US_CTX_INS(US_BUNDLE_NAME).ctx;                                                                  \
    }                                                                                                           \
                                                                                                                \
    extern "C" US_ABI_EXPORT void US_SET_CTX_FUNC(US_BUNDLE_NAME)(cppmicroservices::BundleContextPrivate* ctx); \
    extern "C" US_ABI_EXPORT void US_SET_CTX_FUNC(US_BUNDLE_NAME)(cppmicroservices::BundleContextPrivate* ctx)  \
    {                                                                                                           \
        US_CTX_INS(US_BUNDLE_NAME).usingPubCtx = false;                                                         \
        US_CTX_INS(US_BUNDLE_NAME).ctx = ctx;                                                                   \
    }                                                                                                           \
    extern "C" US_ABI_EXPORT void US_SET_PUBLIC_CTX_FUNC(US_BUNDLE_NAME)(cppmicroservices::BundleContext* ctx); \
    extern "C" US_ABI_EXPORT void US_SET_PUBLIC_CTX_FUNC(US_BUNDLE_NAME)(cppmicroservices::BundleContext* ctx)  \
    {                                                                                                           \
        US_CTX_INS(US_BUNDLE_NAME).usingPubCtx = true;                                                          \
        US_CTX_INS(US_BUNDLE_NAME).ctx = (BundleContextPrivate*)ctx;                                            \
    }

#endif // CPPMICROSERVICES_BUNDLEINITIALIZATION_H_MACRO
#endif // US_BUNDLE_NAME
