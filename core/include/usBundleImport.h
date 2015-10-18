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

#ifndef USBUNDLEIMPORT_H
#define USBUNDLEIMPORT_H

#include <usGlobalConfig.h>

#include <string>

namespace us {

struct BundleActivator;
class BundleContext;

}

/**
 * \ingroup MicroServices
 *
 * \brief Initialize a static bundle.
 *
 * \param _bundle_name The name of the bundle to initialize.
 *
 * This macro initializes the static bundle named \c _bundle_name.
 *
 * If the bundle provides an activator, use the #US_IMPORT_BUNDLE macro instead,
 * to ensure that the activator is called. Do not forget to actually link
 * the static bundle to the importing executable or shared library.
 *
 * \sa US_IMPORT_BUNDLE
 * \sa US_IMPORT_BUNDLE_RESOURCES
 * \sa \ref MicroServices_StaticBundles
 */
#define US_INITIALIZE_STATIC_BUNDLE(_bundle_name)                          \
  extern "C" us::BundleContext* _us_get_bundle_context_instance_ ## _bundle_name (); \
  extern "C" us::BundleContext* _us_set_bundle_context_instance_ ## _bundle_name (); \
  void _dummy_reference_to_ ## _bundle_name ## _bundle_context()           \
  {                                                                        \
    _us_get_bundle_context_instance_ ## _bundle_name();                    \
    _us_set_bundle_context_instance_ ## _bundle_name();                    \
  }

/**
 * \ingroup MicroServices
 *
 * \brief Import a static bundle.
 *
 * \param _bundle_name The name of the bundle to import.
 *
 * This macro imports the static bundle named \c _bundle_name.
 *
 * Inserting this macro into your application's source code will allow you to make use of
 * a static bundle. It will initialize the static bundle and calls its
 * BundleActivator. If the bundle does not provide an activator, use the
 * #US_INITIALIZE_STATIC_BUNDLE macro instead. Do not forget to actually link
 * the static bundle to the importing executable or shared library.
 *
 * Example:
 * \snippet uServices-staticbundles/main.cpp ImportStaticBundleIntoMain
 *
 * \sa US_INITIALIZE_STATIC_BUNDLE
 * \sa US_IMPORT_BUNDLE_RESOURCES
 * \sa \ref MicroServices_StaticBundles
 */
#define US_IMPORT_BUNDLE(_bundle_name)                                     \
  US_INITIALIZE_STATIC_BUNDLE(_bundle_name)                                \
  extern "C" us::BundleActivator* _us_bundle_activator_instance_ ## _bundle_name (); \
  void _dummy_reference_to_ ## _bundle_name ## _activator()                \
  {                                                                        \
    _us_bundle_activator_instance_ ## _bundle_name();                      \
  }

#endif // USBUNDLEREGISTRY_H
