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

#ifndef CPPMICROSERVICES_BUNDLEINITIALIZATION_H
#define CPPMICROSERVICES_BUNDLEINITIALIZATION_H

#include "cppmicroservices/GlobalConfig.h"

#include <atomic>
#include <functional>
#include <mutex>

namespace cppmicroservices
{
    class BundleContext;
    // Using a raw pointer because smart pointers can't go across an extern C interface.
    // THIS IS A TRANSFER OF OWNERSHIP. Make and pass in a copy if you want to retain yours.
    using SetBundleContextHook = std::function<void(BundleContext*)>;
}

// Header file may be included just for the hook data type
#ifdef US_BUNDLE_NAME

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
#    define CPPMICROSERVICES_INITIALIZE_BUNDLE                                                               \
        struct {                                                                                             \
            std::mutex mut;                                                                                  \
            cppmicroservices::BundleContext* ctx = nullptr;                                                  \
        } US_CTX_INS(US_BUNDLE_NAME);                                                                        \
                                                                                                             \
        extern "C" cppmicroservices::BundleContext* US_GET_CTX_FUNC(US_BUNDLE_NAME)(void);                   \
        extern "C" cppmicroservices::BundleContext* US_GET_CTX_FUNC(US_BUNDLE_NAME)()                        \
        {                                                                                                    \
            std::lock_guard<std::mutex> lock(US_CTX_INS(US_BUNDLE_NAME).mut);                                \
            return US_CTX_INS(US_BUNDLE_NAME).ctx;                                                           \
        }                                                                                                    \
                                                                                                             \
        extern "C" US_ABI_EXPORT void US_SET_CTX_FUNC(US_BUNDLE_NAME)(cppmicroservices::BundleContext* ctx); \
        extern "C" US_ABI_EXPORT void US_SET_CTX_FUNC(US_BUNDLE_NAME)(cppmicroservices::BundleContext* ctx)  \
        {                                                                                                    \
            std::lock_guard<std::mutex> lock(US_CTX_INS(US_BUNDLE_NAME).mut);                                \
            if (US_CTX_INS(US_BUNDLE_NAME).ctx) delete US_CTX_INS(US_BUNDLE_NAME).ctx;                       \
            US_CTX_INS(US_BUNDLE_NAME).ctx = ctx;                                                            \
        }

#endif // US_BUNDLE_NAME
#endif // CPPMICROSERVICES_BUNDLEINITIALIZATION_H
