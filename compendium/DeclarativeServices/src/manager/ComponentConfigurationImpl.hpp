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

#ifndef COMPONENTCONFIGURATIONIMPL_HPP__
#define COMPONENTCONFIGURATIONIMPL_HPP__

#include <memory>
#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "../ComponentContextImpl.hpp"
#include "../ConfigurationListenerImpl.hpp"
#include "../metadata/ComponentMetadata.hpp"
#include "ComponentConfiguration.hpp"
#include "ComponentManager.hpp"
#include "ConfigurationManager.hpp"
#include "ConfigurationNotifier.hpp"
#include "ReferenceManager.hpp"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/logservice/LogService.hpp"
#include "cppmicroservices/servicecomponent/detail/ComponentInstance.hpp"
#include "states/ComponentConfigurationState.hpp"

using cppmicroservices::scrimpl::ReferenceManager;
using cppmicroservices::service::component::detail::ComponentInstance;

namespace cppmicroservices::scrimpl
    {
       class SCRExtensionRegistry;

        typedef std::pair<std::shared_ptr<ComponentInstance>, std::shared_ptr<ComponentContextImpl>>
            InstanceContextPair;
        struct ListenerToken final
        {
            ListenerToken(std::string pid, const ListenerTokenId tokenId)
                : pid(std::move(pid))
                , tokenId(tokenId)
            {
            }
            std::string pid;
            ListenerTokenId tokenId;
        };

        /**
         * Abstract class responsible for implementing the state machine
         * for component configurations and some utility methods to create and
         * destroy {@link ComponentInstance} objects. The subclasses of this class
         * are responsible for managing the {@link ComponentInstance} objects
         */
        class ComponentConfigurationImpl
            : public ComponentConfiguration
            , public std::enable_shared_from_this<ComponentConfigurationImpl>
        {
          public:
            /**
             * \throws std::invalid_argument exception if any of the params is a nullptr
             */
            explicit ComponentConfigurationImpl(std::shared_ptr<const metadata::ComponentMetadata> metadata,
                                                Bundle const& bundle,
                                                std::shared_ptr<ComponentRegistry> registry,
                                                std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                                std::shared_ptr<ConfigurationNotifier> configNotifier);
            ComponentConfigurationImpl(ComponentConfigurationImpl const&) = delete;
            ComponentConfigurationImpl(ComponentConfigurationImpl&&) = delete;
            ComponentConfigurationImpl& operator=(ComponentConfigurationImpl const&) = delete;
            ComponentConfigurationImpl& operator=(ComponentConfigurationImpl&&) = delete;
            virtual ~ComponentConfigurationImpl();

            /**
             * Returns all the dependency manager objects associated with this component configuration
             */
            std::vector<std::shared_ptr<ReferenceManager>> GetAllDependencyManagers() const override;

            /**
             * Returns the dependency manager associated with a particular reference
             *
             * \param refName is the name of the reference as described in the component description
             * \return The {@link ReferenceManager} associated with \c refName.
             *         nullptr if no reference manager exists with \c refName
             */
            std::shared_ptr<ReferenceManager> GetDependencyManager(std::string const& refName) const override;

            /** @copydoc ComponentConfiguration::GetServiceReference()
             *
             */
            ServiceReferenceBase GetServiceReference() const override;

            /** @copydoc ComponentConfiguration::GetRegistry()
             *
             */
            std::shared_ptr<ComponentRegistry>
            GetRegistry() const override
            {
                return registry;
            };

            /** @copydoc ComponentConfiguration::GetProperties()
             * These properties must include \c ComponentConstants::COMPONENT_NAME and
             * \c ComponentConstants::COMPONENT_ID
             */
            std::unordered_map<std::string, cppmicroservices::Any> GetProperties() const override;

            /** @copydoc ComponentConfiguration::GetBundle()
             *
             */
            cppmicroservices::Bundle
            GetBundle() const override
            {
                return bundle;
            };

            /** @copydoc ComponentConfiguration::GetId()
             *
             */
            unsigned long
            GetId() const override
            {
                return configID;
            };

            /** @copydoc ComponentConfiguration::GetConfigState()
             *
             */
            ComponentState GetConfigState() const override;

            /**
             * This method returns the {@link ConfigurationNotifier} object
             */
            std::shared_ptr<ConfigurationNotifier>
            GetConfigNotifier() const
            {
                return configNotifier;
            };

            /**
             * This method returns the {@link ComponentMetadata} object created by
             * parsing the component description.
             */
            std::shared_ptr<const metadata::ComponentMetadata>
            GetMetadata() const override
            {
                return metadata;
            };

            
            /**
             * Method to check if this component provides a service
             *
             * \return \c true if this component implements a service interface, \c false otherwise
             */
            bool
            IsServiceProvider()
            {
                return !(metadata->serviceMetadata.interfaces.empty());
            }

            /**
             * Method used to register this component configuration's service. This
             * is invoked by the \c state object
             * @see CCUnsatisfiedState::Register
             *
             * \return true if the registration succeeded, false otherwise.
             */
            bool RegisterService();

            /**
             * Method used to unregister this component configuration's service. This
             * is invoked by the \c state object
             * @see CCSatisfiedState::Deactivate
             * This method is a no-op if the component has not registered a service
             * or if the service is already unregistered.
             */
            void UnregisterService();

            /**
             * Method called while \c REGISTERING this component configuration. Subclasses
             * are responsible for returning a {@link ServiceFactory} object. The
             * returned object is passed to the framework in a call to
             * {@link BundleContext#RegisterService}
             */
            virtual std::shared_ptr<ServiceFactory> GetFactory() = 0;

            /**
             * Method called while \c ACTIVATING this component configuration. Subclasses
             * must implement this method and handle the instance management.
             * \note This function is noexcept. It is not marked as such because gmock does
             *  not support mocking methods with the noexcept keyword.
             */
            virtual std::shared_ptr<ComponentInstance> CreateAndActivateComponentInstance(
                cppmicroservices::Bundle const& bundle)
                = 0;

            /**
             * Method called to modify the configuration properties for this component configuration. Subclasses
             * must implement this method.
              @return boolean indicating whether or not the component instance has provided a Modified method.
             */
            virtual bool ModifyComponentInstanceProperties() = 0;

            /**
             * Method called while \c DEACTIVATING this component configuration. Subclasses
             * must implement this method and handle the instance management.
             */
            virtual void DestroyComponentInstances() = 0;

            /**
             * Method used to kick start the state machine of this configuration
             */
            void Initialize();

            /**
             * Method used to set the \c state of this configuration. This method uses
             * \c std::atomic operations to compare and swap the current state. This
             * method is used by the {@link ComponentConfigurationState} objects to
             * switch this object's state.
             *
             * Note: This method is virtual only for testing purposes
             */
            bool virtual CompareAndSetState(std::shared_ptr<ComponentConfigurationState>* expectedState,
                                            std::shared_ptr<ComponentConfigurationState> desiredState);

            /**
             * Accessor method that returns the state object associated with this object
             */
            std::shared_ptr<ComponentConfigurationState> GetState() const;

            /**
             * Method used to trigger a state change from \c UNSATISFIED_REFERENCE to \c SATISFIED
             * The call is delegated to the current \c state object
             */
            void Register();

            /**
             * Method used to trigger a state change from \c SATISFIED to \c ACTIVE
             * The call is delegated to the current \c state object
             *
             * \return the {@link ComponentInstance} object created as part of the state transition
             */
            std::shared_ptr<ComponentInstance> Activate(Bundle const& usingBundle);

            /**
             * Method used to trigger a state change from \c ACTIVE or \c SATISFIED to \c UNSATISFIED_REFERENCE
             * The call is delegated to the current \c state object
             */
            void Deactivate();

            /**
             * Method used to update the properties of a component instance when a configuration
             * object on which it is dependent changes.
             * @return
             *    - true if the component has a Modified method and is still in the
             *      active state.
             *    - false if the component does not have a Modified method. The
             *      component has been Deactivated
             */
            bool Modified();

            /**
             * SetRegistrationProperties. Sets component properties in registration object.
             */
            void SetRegistrationProperties();

            /**
             * Method called to stop the service trackers associated with this configuration's reference managers
             */
            void Stop();

            /**
             * Returns the logger object associated with the current runtime
             */
            std::shared_ptr<logservice::LogService>
            GetLogger() const
            {
                return logger;
            }

            /**
             * Method called while performing a dynamic rebind. Subclasses must
             * implement this method to call a service component's bind method.
             * \param refName is the name of the reference as defined in the SCR JSON
             * \param ref is the service reference to the target service to bind. A default
             *  constructed \c ServiceReferenceBase denotes that there is no service to bind.
             */
            virtual void BindReference(std::string const& refName, ServiceReferenceBase const& ref) = 0;

            /**
             * Method called while performing a dynamic rebind. Subclasses must
             * implement this method to call a service component's unbind method.
             * \param refName is the name of the reference as defined in the SCR JSON
             * \param ref is the service reference to the target service to unbind. A default
             *  constructed \c ServiceReferenceBase denotes that there is no service to unbind.
             */
            virtual void UnbindReference(std::string const& refName, ServiceReferenceBase const& ref) = 0;

          protected:
            /**
             * This method is responsible for creating a {@link ComponentInstance} object
             */
            std::shared_ptr<ComponentInstance> CreateComponentInstance();

            /**
             * Helper function used by sub-classes to create and activate a {@link ComponentInstance} object
             */
            InstanceContextPair CreateAndActivateComponentInstanceHelper(cppmicroservices::Bundle const& bundle);

            /**
             * Sets the function pointers used to create and delete a {@link ComponentInstance} object
             */
            void
            SetComponentInstanceCreateDeleteMethods(std::function<ComponentInstance*(void)> newFunc,
                                                    std::function<void(ComponentInstance*)> deleteFunc)
            {
                newCompInstanceFunc = newFunc;
                deleteCompInstanceFunc = deleteFunc;
            }

            /**
             * Utility method used in tests to prepare the object for a test point.
             * Note: Do NOT use this method in production code.
             */
            void
            SetState(std::shared_ptr<ComponentConfigurationState> const& newState)
            {
                auto oldState = ComponentConfigurationImpl::GetState();
                ComponentConfigurationImpl::CompareAndSetState(&oldState, newState);
            }

          private:
            /**
             * Observer callback method. This method is registered with the dependency
             * managers. When a reference becomes \c SATISFIED or \c UNSATISFIED, this
             * method is called by the dependency manager to notify the configuration object.
             */
            void RefChangedState(RefChangeNotification const& notification);

            /**
             * Utility method with actions to be performed when a reference is satisfied.
             * This method is called from {@link #RefChangedState} when
             * {@link RefChangeNotification#senderState} is \c SATISFIED
             */
            void RefSatisfied(std::string const& refName);

            /**
             * Utility method with actions to be performed when a reference is satisfied
             * This method is called from {@link #RefChangedState} when
             * {@link RefChangeNotification#senderState} is \c UNSATISFIED
             */
            void RefUnsatisfied(std::string const& refName);

            /**
             * Utility method called to determine if all references for the component
             * are satisfied. This method is called from {@link #ConfigChangedState} when
             * Configuration objects are satisfied.
             */
            bool AreReferencesSatisfied() const noexcept;

            /**
             * Observer callback method. This method is registered with the ConfigurationListener.
             * When a configuration object is \c updated or \c deleted, this
             * method is called by the ConfigurationListener to process the change to the
             * configuration object.
             */

            void ConfigChangedState(ConfigChangeNotification const& notification);

            /**
             * Method is responsible for loading the bundle and populating the function
             * objects \c newCompInstanceFunc & \c deleteCompInstanceFunc used to create
             * and destroy the {@link ComponentInstance} objects from a bundle.
             */
            void LoadComponentCreatorDestructor();

            /**
             * Friends used in unittests
             */
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyCtor);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyInitializeForImmediateComponent);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyInitializeForDelayedComponent);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyRegister);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyActivate_Success);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyActivate_Failure);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyDeactivate);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyConcurrentRegisterDeactivate);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyConcurrentActivateDeactivate);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyRefSatisfied);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyRefUnsatisfied);
            FRIEND_TEST(ComponentConfigurationImplTest, VerifyStateChangeDelegation);
            FRIEND_TEST(ComponentConfigurationImplTest, TestGetDependencyManagers);

            unsigned long configID; ///< unique Id for the component configuration
            static std::atomic<unsigned long>
                idCounter; ///< used to assign unique identifiers to component configurations
            const std::shared_ptr<const metadata::ComponentMetadata> metadata; ///< component description
            Bundle bundle;                                     ///< bundle this component configuration belongs to
            const std::shared_ptr<ComponentRegistry> registry; ///< component registry of the runtime
            const std::shared_ptr<cppmicroservices::logservice::LogService>
                logger; ///< logger used for reporting errors/execptions
            std::unique_ptr<RegistrationManager>
                regManager; ///< registration manager used to manage registration/unregistration of the service provided
                            ///< by this component
            std::unordered_map<std::string, std::shared_ptr<ReferenceManager>>
                referenceManagers; ///< map of all the reference managers
            std::unordered_map<std::shared_ptr<ReferenceManager>, ListenerTokenId>
                referenceManagerTokens; ///< map of the listener tokens received from the reference managers
            std::shared_ptr<ConfigurationManager> configManager;   ///< manages configuration objects
            std::shared_ptr<ConfigurationNotifier> configNotifier; // to get updates for configuration objects
            std::vector<std::shared_ptr<ListenerToken>>
                configListenerTokens; ///< vector of the listener tokens received from the config manager
            std::shared_ptr<ComponentConfigurationState> state; ///< only modified using std::atomic operations
            std::function<ComponentInstance*(void)>
                newCompInstanceFunc; ///< extern C function to create a new instance {@link ComponentInstance} class
                                     ///< from the component's bundle
            std::function<void(ComponentInstance*)>
                deleteCompInstanceFunc; ///< extern C function to delete an instance of the {@link ComponentInstance}
                                        ///< class from the component's bundle
        };
    } // namespace cppmicroservices::scrimpl
#endif /* COMPONENTCONFIGURATIONIMPL_HPP__ */
