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

#ifndef _SERVICE_IMPL_HPP_
#define _SERVICE_IMPL_HPP_

#include <mutex>

#include "cppmicroservices/servicecomponent/ComponentContext.hpp"
#include "TestInterfaces/Interfaces.hpp"

using ComponentContext = cppmicroservices::service::component::ComponentContext;

namespace sample {

class ServiceComponentDynamicGreedyMandatoryUnary final : public test::Interface2
{
public:
  ServiceComponentDynamicGreedyMandatoryUnary() = default;
  ~ServiceComponentDynamicGreedyMandatoryUnary() = default;
  virtual std::string ExtendedDescription() override;
  
  void Activate(const std::shared_ptr<ComponentContext>&);
  void Deactivate(const std::shared_ptr<ComponentContext>&);

  void Bindfoo(const std::shared_ptr<test::Interface1>&);
  void Unbindfoo(const std::shared_ptr<test::Interface1>&);
private:
  std::shared_ptr<test::Interface1> foo;
  std::mutex fooMutex;
};

} // namespaces

#endif // _SERVICE_IMPL_HPP_