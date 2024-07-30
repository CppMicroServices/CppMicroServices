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

#ifndef CppMicroServices_CM_Configuration_hpp
#define CppMicroServices_CM_Configuration_hpp

#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/ThreadpoolSafeFuture.h"
#include <future>
#include <string>

namespace cppmicroservices
{
    namespace service
    {
        namespace cm
        {

            /**
             \defgroup gr_configuration Configuration
             \brief Groups Configuration class related symbols.
             */

            /**
             * \ingroup gr_configuration
             *
             * The Configuration object (normally obtained as a std::shared_ptr<Configuration>)
             * is the principal means for clients of ConfigurationAdmin to inspect or update the
             * Configuration of a given service or service factory.
             */
            class Configuration
            {
              public:
                virtual ~Configuration() noexcept = default;

                /**
                 * Get the PID of this Configuration.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed
                 *
                 * @return the PID of this Configuration
                 */
                virtual std::string GetPid() const = 0;

                /**
                 * Get the Factory PID which is responsible for this Configuration. If this
                 * Configuration does not belong to any Factory, returns an empty string.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed
                 *
                 * @return the Factory PID associated with this Configuration, if applicable
                 */
                virtual std::string GetFactoryPid() const = 0;

                /**
                 * Get the properties of this Configuration. Returns a copy.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed
                 *
                 * @return the properties of this Configuration
                 */
                virtual AnyMap GetProperties() const = 0;

                /**
                 * Get the change count. Each Configuration must maintain a change counter that
                 * is incremented with a positive value every time the configuration is updated
                 * and its properties are stored. The counter must be incremented before the targets
                 * are updated and events are sent out.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed
                 *
                 * @return A monotonically increasing value reflecting changes in this Configuration.
                 */
                virtual unsigned long GetChangeCount() const = 0;

                /**
                 * Update the properties of this Configuration. Invoking this method will trigger the
                 * ConfigurationAdmin impl to push the updated properties to any ManagedService /
                 * ManagedServiceFactory / ConfigurationListener which has a matching PID / Factory PID.
                 *
                 * If the properties are empty, the Configuration will not be removed, but instead
                 * updated with an empty properties map.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed
                 *
                 * @param properties The properties to update this Configuration with.
                 *
                 * @remarks The shared_ptr<ThreadpoolSafeFuture> returned can contain a
                 * cppmicroservices::SecurityException if the Configuration caused a bundle's shared library to be
                 * loaded and the bundle failed a security check.
                 *
                 * @return a shared_ptr<ThreadpoolSafeFuture> which can be used to wait for the asynchronous
                 * operation that pushed the update to a ManagedService, ManagedServiceFactory or
                 * ConfigurationListener to complete. This can be safely done from the same asyncWorkService on which
                 * the update is done. If an exception occurs during the execution of the service component's Modified
                 * method, this exception is intercepted and logged by Declarative Services. This exception is not
                 * returned in the shared_ptr<ThreadpoolSafeFuture>.
                 */
                virtual std::shared_ptr<ThreadpoolSafeFuture> SafeUpdate(
                    AnyMap properties = AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS })
                    = 0;

                /**
                 * Same as SafeUpdate() except:
                 * @return a std::shared_future<void> that is unsafe to wait on from within a thread allocated to the
                 * AsyncWorkService
                 */
                virtual std::shared_future<void> Update(AnyMap properties
                                                        = AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS })
                    = 0;

                /**
                 * Update the properties of this Configuration if they differ from the current properties.
                 * Invoking this method will trigger the ConfigurationAdmin impl to push the updated
                 * properties to any ManagedService / ManagedServiceFactory / ConfigurationListener which has a matching
                 * PID / Factory PID, but only if the properties differ from the current properties. It will return true
                 * in this case, and false otherwise.
                 *
                 * If the properties are empty, the Configuration will not be removed, but instead
                 * updated with an empty properties map, unless it already had empty properties.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed
                 *
                 * @param properties The properties to update this Configuration with (if they differ)
                 *
                 * @remarks The shared_ptr<ThreadpoolSafeFuture> returned can contain a
                 * cppmicroservices::SecurityException if the Configuration caused a bundle's shared library to be
                 * loaded and the bundle failed a security check.
                 *
                 * @return std::pair<boolean, std::shared_ptr<ThreadpoolSafeFuture>> The boolean indicates whether
                 * the properties were updated or not. The shared_ptr<ThreadpoolSafeFuture> allows access to the result
                 * of the asynchronous operation that pushed the update operation to a ManagedService,
                 * ManagedServiceFactory or ConfigurationListener. This can be safely done from the same
                 * asyncWorkService on which the update is done. If an exception occurs during the execution of the
                 * service component's Modified method, this exception is intercepted and logged by Declarative
                 * Services. This exception is not returned in the shared_ptr<ThreadpoolSafeFuture>.
                 */
                virtual std::pair<bool, std::shared_ptr<ThreadpoolSafeFuture>> SafeUpdateIfDifferent(
                    AnyMap properties = AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS })
                    = 0;

                /**
                 * Same as SafeUpdateIfDifferent() except:
                 * @return a std::shared_future<void> that is unsafe to wait on from within a thread allocated to the
                 * AsyncWorkService
                 */
                virtual std::pair<bool, std::shared_future<void>> UpdateIfDifferent(
                    AnyMap properties = AnyMap { AnyMap::UNORDERED_MAP_CASEINSENSITIVE_KEYS })
                    = 0;

                /**
                 * Remove this Configuration from ConfigurationAdmin. This will trigger a push to any
                 * ConfigurationListener. This will also trigger the ConfigurationAdmin
                 * implementation to update any corresponding ManagedService with an empty AnyMap. Any
                 * corresponding ManagedServiceFactory will have its Removed method invoked with the
                 * corresponding PID.
                 *
                 * @throws std::runtime_error if this Configuration object has been Removed already
                 *
                 * @return a shared_ptr<ThreadpoolSafeFuture> to access the result of the asynchronous operation
                 * that pushed the remove operation to a ManagedService, ManagedServiceFactory or
                 * ConfigurationListener. This can be safely done from the same
                 * asyncWorkService on which the update is done. If an exception occurs during the execution
                 * of the service component's Modified method, this exception is intercepted and
                 * logged by Declarative Services. This exception is not returned in the
                 * shared_ptr<ThreadpoolSafeFuture>.
                 */
                virtual std::shared_ptr<ThreadpoolSafeFuture> SafeRemove() = 0;

                /**
                 * Same as SafeRemove() except:
                 * @return a std::shared_future<void> that is unsafe to wait on from within a thread allocated to the
                 * AsyncWorkService
                 */
                virtual std::shared_future<void> Remove() = 0;
            };
        } // namespace cm
    }     // namespace service
} // namespace cppmicroservices
#endif // Configuration_hpp
