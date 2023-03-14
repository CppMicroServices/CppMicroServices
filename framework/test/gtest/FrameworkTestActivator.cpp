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

#include "FrameworkTestActivator.h"

#include "cppmicroservices/BundleImport.h"

namespace cppmicroservices
{

    FrameworkTestActivator* FrameworkTestActivator::m_Instance = nullptr;

    FrameworkTestActivator::FrameworkTestActivator() : m_StartCalled(false) {}

    bool
    FrameworkTestActivator::StartCalled()
    {
        return m_Instance ? m_Instance->m_StartCalled : false;
    }

    void
    FrameworkTestActivator::Start(BundleContext)
    {
        this->m_Instance = this;
        this->m_StartCalled = true;
    }

    void
    FrameworkTestActivator::Stop(BundleContext)
    {
        this->m_Instance = nullptr;
    }
} // namespace cppmicroservices

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(cppmicroservices::FrameworkTestActivator)

#ifndef US_BUILD_SHARED_LIBS
CPPMICROSERVICES_IMPORT_BUNDLE(main)
#endif
