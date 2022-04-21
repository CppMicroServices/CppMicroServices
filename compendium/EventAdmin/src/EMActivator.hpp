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

#ifndef EMACTIVATOR_HPP
#define EMACTIVATOR_HPP

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/ListenerToken.h"

#include "EventAdminImpl.hpp"

namespace cppmicroservices {
namespace emimpl {
class EMActivator final : public cppmicroservices::BundleActivator
{
public:
  EMActivator() = default;
  EMActivator(const EMActivator&) = delete;
  EMActivator(EMActivator&&) = delete;
  EMActivator& operator=(const EMActivator&) = delete;
  EMActivator& operator=(EMActivator&&) = delete;
  ~EMActivator() override = default;

  void Start(cppmicroservices::BundleContext context) override;
  void Stop(cppmicroservices::BundleContext context) override;

private:
  cppmicroservices::BundleContext runtimeContext;
  std::shared_ptr<EventAdminImpl> eventAdminImpl;
  cppmicroservices::ServiceRegistration<
    cppmicroservices::service::em::EventAdmin>
    eventAdminReg;
};
} // emimpl
} // cppmicroservices

#endif // EMACTIVATOR_HPP
