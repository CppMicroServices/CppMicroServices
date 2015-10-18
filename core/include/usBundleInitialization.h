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

#ifndef US_BUNDLE_NAME
#error Missing US_BUNDLE_NAME preprocessor define
#endif

#ifndef USBUNDLEINITIALIZATION_H
#define USBUNDLEINITIALIZATION_H

#include <usGlobalConfig.h>

namespace us {
class BundleContext;
}

/**
 * \ingroup MicroServices
 *
 * \brief Creates initialization code for a bundle.
 *
 * Each bundle which wants to register itself with the CppMicroServices library
 * has to put a call to this macro in one of its source files. Further, the bundle's
 * source files must be compiled with the \c US_BUNDLE_NAME pre-processor definition
 * set to a bundle-unique identifier.
 *
 * Calling the \c US_INITIALIZE_BUNDLE macro will initialize the bundle for use with
 * the CppMicroServices library, using a default auto-load directory named after the
 * \c US_BUNDLE_NAME definition.
 *
 * \sa MicroServices_AutoLoading
 *
 * \remarks If you are using CMake, consider using the provided CMake macro
 * <code>usFunctionGenerateBundleInit()</code>.
 */
#define US_INITIALIZE_BUNDLE                                                                                                   \
  us::BundleContext* US_CONCAT(_us_bundle_context_instance_, US_BUNDLE_NAME) = 0;                            \
                                                                                                                               \
  extern "C" US_ABI_EXPORT us::BundleContext* US_CONCAT(_us_get_bundle_context_instance_, US_BUNDLE_NAME) () \
  {                                                                                                                            \
    return US_CONCAT(_us_bundle_context_instance_, US_BUNDLE_NAME);                                                            \
  }                                                                                                                            \
                                                                                                                               \
  extern "C" US_ABI_EXPORT void US_CONCAT(_us_set_bundle_context_instance_, US_BUNDLE_NAME) (us::BundleContext* ctx) \
  {                                                                                                                                    \
    US_CONCAT(_us_bundle_context_instance_, US_BUNDLE_NAME) = ctx;                                                                     \
  }


#endif // USBUNDLEINITIALIZATION_H
