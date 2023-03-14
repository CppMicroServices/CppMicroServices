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
#include <cppmicroservices/AnyMap.h>
#include <cppmicroservices/cm/ManagedService.hpp>
#include <cppmicroservices/servicecomponent/ComponentContext.hpp>

#include "TestInterfaces/Interfaces.hpp"

#include <mutex>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {
            namespace test
            {

                class TestManagedServiceImpl
                    : public ::test::TestManagedServiceInterface
                    , public cppmicroservices::service::cm::ManagedService
                {
                  public:
                    TestManagedServiceImpl();

                    virtual ~TestManagedServiceImpl();

                    void Updated(AnyMap const& properties) override;

                    void Activate(std::shared_ptr<cppmicroservices::service::component::ComponentContext> const&);
                    void
                    Deactivate(std::shared_ptr<cppmicroservices::service::component::ComponentContext> const&)
                    {
                    }

                    int getCounter() override;

                  private:
                    int m_counter;
                    std::mutex m_counterMtx;
                };

            } // namespace test
        }     // namespace cm
    }         // namespace service
} // namespace cppmicroservices
