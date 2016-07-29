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

#ifndef USBUNDLECONTEXTPRIVATE_H_
#define USBUNDLECONTEXTPRIVATE_H_

#include "usLog_p.h"
#include "usThreads_p.h"
#include <usCoreExport.h>

#include <atomic>
#include <memory>

namespace us {

class BundleContext;
class BundlePrivate;

class BundleContextPrivate : public MultiThreaded<>, public std::enable_shared_from_this<BundleContextPrivate>
{

public:

  BundleContextPrivate(BundlePrivate* bundle);

  bool IsValid() const;
  void CheckValid() const;

  void Invalidate();

  BundlePrivate* bundle;

  /**
   * Is bundle context valid.
   */
  std::atomic<bool> valid;

};

// The following method is exported for the GetBundleContext() method
US_Core_EXPORT BundleContext MakeBundleContext(BundleContextPrivate* d);
BundleContext MakeBundleContext(const std::shared_ptr<BundleContextPrivate>& d);
std::shared_ptr<BundleContextPrivate> GetPrivate(const BundleContext& c);

}

#endif
