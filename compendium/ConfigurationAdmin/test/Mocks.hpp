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

#ifndef MOCKS_HPP
#define MOCKS_HPP

#include "gmock/gmock.h"

#include "cppmicroservices/cm/ManagedService.hpp"
#include "cppmicroservices/cm/ManagedServiceFactory.hpp"
#include "cppmicroservices/logservice/LogService.hpp"

#include "../src/ConfigurationAdminPrivate.hpp"

namespace cppmicroservices {
  namespace cmimpl {

    /**
     * This class is used in tests where the logger is required and the test
     * needs to verify what is sent to the logger
     */
    class MockLogger : public cppmicroservices::logservice::LogService
    {
    public:
      MOCK_METHOD2(Log, void(cppmicroservices::logservice::SeverityLevel, const std::string&));
      MOCK_METHOD3(Log, void(cppmicroservices::logservice::SeverityLevel, const std::string&, const std::exception_ptr));
      MOCK_METHOD3(Log, void(const cppmicroservices::ServiceReferenceBase&, cppmicroservices::logservice::SeverityLevel, const std::string&));
      MOCK_METHOD4(Log, void(const cppmicroservices::ServiceReferenceBase&, cppmicroservices::logservice::SeverityLevel, const std::string&, const std::exception_ptr));
    };

    /**
     * This class is used in tests where the logger is required but the test
     * does not care if anything is actually sent to the logger
     */
    class FakeLogger : public cppmicroservices::logservice::LogService
    {
    public:
      void Log(cppmicroservices::logservice::SeverityLevel, const std::string&) override {}
      void Log(cppmicroservices::logservice::SeverityLevel, const std::string&, const std::exception_ptr) override {}
      void Log(const ServiceReferenceBase&, cppmicroservices::logservice::SeverityLevel, const std::string&) override {}
      void Log(const ServiceReferenceBase&, cppmicroservices::logservice::SeverityLevel, const std::string&, const std::exception_ptr) override {}
    };

    /**
     * This class is used in tests which need to verify that the Updated method of
     * a ManagedService is called as expected. The mock can be registered with the
     * Framework to "inject" it into the class under test.
     */
    class MockManagedService : public cppmicroservices::service::cm::ManagedService {
    public:
      MOCK_METHOD1(Updated, void(const AnyMap&));
    };

    /**
     * This class is used in tests which need to verify that the Updated and Removed
     * methods of a ManagedService are called as expected. The mock can be registered
     * with the Framework to "inject" it into the class under test.
     */
    class MockManagedServiceFactory : public cppmicroservices::service::cm::ManagedServiceFactory {
    public:
      MOCK_METHOD2(Updated, void(const std::string&, const AnyMap&));
      MOCK_METHOD1(Removed, void(const std::string&));
    };

    /**
     * This class is used in tests of classes which would normally callback to a
     * ConfigurationAdminPrivate. It allows the test to verify that the correct
     * methods are invoked as expected.
     */
    class MockConfigurationAdminPrivate : public ConfigurationAdminPrivate {
    public:
      MOCK_METHOD1(AddConfigurations, std::vector<ConfigurationAddedInfo>(std::vector<metadata::ConfigurationMetadata>));
      MOCK_METHOD1(RemoveConfigurations, void(std::vector<ConfigurationAddedInfo>));
      MOCK_METHOD1(NotifyConfigurationUpdated, void(const std::string&));
      MOCK_METHOD2(NotifyConfigurationRemoved, void(const std::string&, std::uintptr_t));
    };
  }
}

#endif /* MOCKS_HPP */
