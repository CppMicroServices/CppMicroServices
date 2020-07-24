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

#include <cassert>
#include <sstream>

#include "ConfigurationImpl.hpp"

namespace {
  constexpr auto REMOVED_EXCEPTION_MESSAGE = "This Configuration has been Removed";
}

namespace cppmicroservices {
  namespace cmimpl {

    ConfigurationImpl::ConfigurationImpl(ConfigurationAdminPrivate* configAdmin,
                                         std::string thePid,
                                         std::string theFactoryPid,
                                         AnyMap props)
    : configAdminImpl(configAdmin)
    , pid(std::move(thePid))
    , factoryPid(std::move(theFactoryPid))
    , properties(std::move(props))
    , changeCount{1u}
    , removed{false}
    {
      assert(configAdminImpl != nullptr && "Invalid ConfigurationAdminPrivate pointer");
    }

    std::string ConfigurationImpl::GetPid() const
    {
      std::lock_guard<std::mutex> lk{propertiesMutex};
      if (removed)
      {
        throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
      }
      return pid;
    }

    std::string ConfigurationImpl::GetFactoryPid() const
    {
      std::lock_guard<std::mutex> lk{propertiesMutex};
      if (removed)
      {
        throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
      }
      return factoryPid;
    }

    AnyMap ConfigurationImpl::GetProperties() const
    {
      std::lock_guard<std::mutex> lk{propertiesMutex};
      if (removed)
      {
        throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
      }
      return properties;
    }

    void ConfigurationImpl::Update(AnyMap newProperties)
    {
      {
        std::lock_guard<std::mutex> lk{propertiesMutex};
        if (removed)
        {
          throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
        }
        properties = std::move(newProperties);
        ++changeCount;
      }
      std::lock_guard<std::mutex> lk{configAdminMutex};
      if (configAdminImpl)
      {
        configAdminImpl->NotifyConfigurationUpdated(pid);
      }
    }

    bool ConfigurationImpl::UpdateIfDifferent(AnyMap newProperties)
    {
      const auto updated = UpdateWithoutNotificationIfDifferent(std::move(newProperties)).first;
      if (!updated)
      {
        return false;
      }
      std::lock_guard<std::mutex> lk{configAdminMutex};
      if (configAdminImpl)
      {
        configAdminImpl->NotifyConfigurationUpdated(pid);
      }
      return true;
    }

    void ConfigurationImpl::Remove()
    {
      {
        std::lock_guard<std::mutex> lk{propertiesMutex};
        if (removed)
        {
          throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
        }
        removed = true;
      }
      std::lock_guard<std::mutex> lk{configAdminMutex};
      if (configAdminImpl)
      {
        configAdminImpl->NotifyConfigurationRemoved(pid, reinterpret_cast<std::uintptr_t>(this));
        configAdminImpl = nullptr;
      }
    }

    std::pair<bool, unsigned long> ConfigurationImpl::UpdateWithoutNotificationIfDifferent(AnyMap newProperties)
    {
      std::lock_guard<std::mutex> lk{propertiesMutex};
      if (removed)
      {
        throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
      }
      std::ostringstream existingProps;
      std::ostringstream newProps;
      cppmicroservices::any_value_to_string(existingProps, properties);
      cppmicroservices::any_value_to_string(newProps, newProperties);
      if (existingProps.str() == newProps.str())
      {
        return std::pair<bool, unsigned long>{false, 0u};
      }
      properties = std::move(newProperties);
      return std::pair<bool, unsigned long>{true, ++changeCount};
    }

    bool ConfigurationImpl::RemoveWithoutNotificationIfChangeCountEquals(unsigned long expectedChangeCount)
    {
      std::lock_guard<std::mutex> lk{propertiesMutex};
      if (removed)
      {
        throw std::runtime_error(REMOVED_EXCEPTION_MESSAGE);
      }
      if (expectedChangeCount == changeCount)
      {
        removed = true;
        return true;
      }
      return false;
    }

    void ConfigurationImpl::Invalidate()
    {
      std::lock_guard<std::mutex> lk{configAdminMutex};
      configAdminImpl = nullptr;
    }
  } // cmimpl
} // cppmicroservices
