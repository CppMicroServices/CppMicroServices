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

#ifndef CPPMICROSERVICES_SERVICEREGISTRY_H
#define CPPMICROSERVICES_SERVICEREGISTRY_H

#include "cppmicroservices/ServiceInterface.h"
#include "cppmicroservices/ServiceRegistration.h"
#include "cppmicroservices/detail/Threads.h"

namespace cppmicroservices
{

    class CoreBundleContext;
    class BundlePrivate;
    class Properties;

    /**
     * Here we handle all the CppMicroServices services that are registered.
     */
    class ServiceRegistry : private detail::MultiThreaded<>
    {

      public:
        void Clear();

        /**
         * Creates a new ServiceProperties object containing <code>in</code>
         * with the keys converted to lower case.
         *
         * @param classes A list of class names which will be added to the
         *        created ServiceProperties object under the key
         *        BundleConstants::OBJECTCLASS.
         * @param sid A service id which will be used instead of a default one.
         */
        static Properties CreateServiceProperties(ServiceProperties const& in,
                                                  std::vector<std::string> const& classes = std::vector<std::string>(),
                                                  bool isFactory = false,
                                                  bool isPrototypeFactory = false,
                                                  long sid = -1);

        using MapServiceClasses = std::unordered_map<ServiceRegistrationBase, std::vector<std::string>>;
        using MapClassServices = std::unordered_map<std::string, std::vector<ServiceRegistrationBase>>;

        /**
         * All registered services in the current framework.
         * Mapping of registered service to class names under which
         * the service is registerd.
         */
        MapServiceClasses services;

        std::vector<ServiceRegistrationBase> serviceRegistrations;

        /**
         * Mapping of classname to registered service.
         * The List of registered services are ordered with the highest
         * ranked service first.
         */
        MapClassServices classServices;

        CoreBundleContext* core;

        ServiceRegistry(ServiceRegistry const&) = delete;
        ServiceRegistry& operator=(ServiceRegistry const&) = delete;

        ServiceRegistry(CoreBundleContext* coreCtx);

        /**
         * Register a service in the framework wide register.
         *
         * @param bundle The bundle registering the service.
         * @param classes The class names under which the service can be located.
         * @param service The service object.
         * @param properties The properties for this service.
         * @return A ServiceRegistration object.
         * @exception std::invalid_argument If one of the following is true:
         * <ul>
         * <li>The service object is 0.</li>
         * <li>The service parameter is not a ServiceFactory or an
         * instance of all the named classes in the classes parameter.</li>
         * </ul>
         */
        ServiceRegistrationBase RegisterService(BundlePrivate* bundle,
                                                InterfaceMapConstPtr const& service,
                                                ServiceProperties const& properties);

        /**
         * Reorder registered services. Call this method if the ranking for
         * a service registration has changed
         *
         * @param classes is the list of classes whose entries need to be reordered
         */
        void UpdateServiceRegistrationOrder(std::vector<std::string> const& classes);

        /**
         * Get all services implementing a certain class.
         * Only used internally by the framework.
         *
         * @param clazz The class name of the requested service.
         * @return A sorted list of {@link ServiceRegistrationPrivate} objects.
         */
        void Get(std::string const& clazz, std::vector<ServiceRegistrationBase>& serviceRegs) const;

        /**
         * Get a service implementing a certain class.
         *
         * @param bundle The bundle requesting reference
         * @param clazz The class name of the requested service.
         * @return A {@link ServiceReference} object.
         */
        ServiceReferenceBase Get(BundlePrivate* bundle, std::string const& clazz) const;

        /**
         * Get all services implementing a certain class and then
         * filter these with a property filter.
         *
         * @param clazz The class name of requested service.
         * @param filter The property filter.
         * @param bundle The bundle requesting reference.
         * @return A list of {@link ServiceReference} object.
         */
        void Get(std::string const& clazz,
                 std::string const& filter,
                 BundlePrivate* bundle,
                 std::vector<ServiceReferenceBase>& serviceRefs) const;

        /**
         * Remove a registered service.
         *
         * @param sr The ServiceRegistration object that is registered.
         */
        void RemoveServiceRegistration(ServiceRegistrationBase const& sr);

        /**
         * Get all services that a bundle has registered.
         *
         * @param p The bundle
         * @return A set of {@link ServiceRegistration} objects
         */
        void GetRegisteredByBundle(BundlePrivate* m, std::vector<ServiceRegistrationBase>& serviceRegs) const;

        /**
         * Get all services that a bundle uses.
         *
         * @param bundle The bundle
         * @return A set of {@link ServiceRegistration} objects
         */
        void GetUsedByBundle(BundlePrivate* bundle, std::vector<ServiceRegistrationBase>& serviceRegs) const;

      private:
        friend class ServiceHooks;
        friend class ServiceRegistrationBase;

        void RemoveServiceRegistration_unlocked(ServiceRegistrationBase const& sr);

        void Get_unlocked(std::string const& clazz, std::vector<ServiceRegistrationBase>& serviceRegs) const;

        void Get_unlocked(std::string const& clazz,
                          std::string const& filter,
                          BundlePrivate* bundle,
                          std::vector<ServiceReferenceBase>& serviceRefs) const;
    };
} // namespace cppmicroservices

#endif // CPPMICROSERVICES_SERVICEREGISTRY_H
