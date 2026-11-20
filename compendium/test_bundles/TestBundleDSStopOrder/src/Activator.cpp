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

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"

#include <stdexcept>

namespace sample
{
    /**
     * The framework calls Stop() after BUNDLE_STOPPING has been delivered to the bundle
     * listeners and before it cleans up the bundle. By then DS must have disposed this
     * bundle's component, so it must not have any services registered any more.
     */
    class Activator : public cppmicroservices::BundleActivator
    {
      public:
        void
        Start(cppmicroservices::BundleContext) override
        {
        }

        void
        Stop(cppmicroservices::BundleContext context) override
        {
            if (!context.GetBundle().GetRegisteredServices().empty())
            {
                throw std::runtime_error("component service still registered when the bundle activator stopped");
            }
        }
    };
} // namespace sample

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(sample::Activator)
