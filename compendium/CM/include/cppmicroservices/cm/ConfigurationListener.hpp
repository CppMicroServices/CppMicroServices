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

#ifndef CppMicroServices_service_CM_ConfigurationListener_hpp
#define CppMicroServices_service_CM_ConfigurationListener_hpp

#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/cm/ConfigurationAdmin.hpp"

namespace cppmicroservices {
namespace service {
namespace cm {
/**
         * The ConfigurationEventType is passed to a Configuration Listener to 
         * identify the type of ConfigurationEvent that has occurred.
         */
enum class ConfigurationEventType
{
  /* The ConfigurationEvent type for when a Configuration object has been 
             * updated 
             */
  CM_UPDATED = 1,

  /* The ConfigurationEvent type for when a Configuration object has been 
             * removed 
             */

  CM_REMOVED = 2
};

/**
	     * The ConfigurationEvent object is passed to the ConfigurationListener when 
	     * the configuration for any service is updated or removed by ConfigurationAdmin
	     */
class ConfigurationEvent
{
public:
  ConfigurationEvent(const ServiceReference<ConfigurationAdmin> _configAdmin,
                     const ConfigurationEventType _type,
                     const std::string _factoryPid,
                     const std::string _pid)
    : configAdmin(_configAdmin)
    , type(std::move(_type))
    , factoryPid(std::move(_factoryPid))
    , pid(std::move(_pid))
  {}

  /**
   * Get the ServiceReference object of the Configuration Admin Service that created 
   * this event.
   */
  const ServiceReference<ConfigurationAdmin>& getReference() const  { return configAdmin; }
  /**
   * Get the PID of this ConfigurationEvent.
   */
  const std::string getPid() const { return pid; }
  /**
   * Get the Factory PID which is responsible for this Configuration.
   */
  const std::string getFactoryPid() const { return factoryPid; }
  /**
   * Get the type of this Configuration.
   */
  const ConfigurationEventType getType() const { return type; }

private:
  const ServiceReference<ConfigurationAdmin>& configAdmin;
  const std::string pid;
  const std::string factoryPid;
  const ConfigurationEventType type;
};
/**
 * The ConfigurationListener interface is the interface that services should implement
 * to receive updates from the ConfigurationAdmin implementation for all configuration 
 * updates. This interface is used by Declarative Services to receive updates to 
 * configuration objects for services that are managed by DS. 
 */
class ConfigurationListener
{
public:
  /**
   * Called whenever the Configuration for any service is updated or removed from ConfigurationAdmin,
   * and when the ConfigurationListener is first registered with the Framework, to provide the initial Configuration.
   */
  virtual void configurationEvent(ConfigurationEvent& event) = 0;
};
}
}
}
#endif /* CppMicroServices_service_CM_ConfigurationListener_hpp */
