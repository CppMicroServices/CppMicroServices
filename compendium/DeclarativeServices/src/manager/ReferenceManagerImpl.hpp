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

#ifndef REFERENCEMANAGERIMPL_HPP
#define REFERENCEMANAGERIMPL_HPP

#include <mutex>

#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "cppmicroservices/GuardedObject.h"
#include "ReferenceManager.hpp"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"

namespace cppmicroservices
{
    namespace scrimpl
    {

        using RefMgrListenerMap
            = std::unordered_map<cppmicroservices::ListenerTokenId, std::function<void(RefChangeNotification const&)>>;
        /**
         * This class is responsible for tracking a service reference (dependency)
         * and based on the policy criteria, notify the listener about the state
         * changes of the reference
         *
         * Note that this is NOT the final implementation class. This class was made in order to
         * be able to mock this implementation for testing purposes. The "ReferenceManagerImpl"
         * class at the end of this file is what's instantiated and used by the rest of the
         * system.
         */
        class ReferenceManagerBaseImpl
            : public ReferenceManager
            , public cppmicroservices::ServiceTrackerCustomizer<void>
        {
          public:
            /**
             * Constructor
             *
             * \param metadata - the reference description as specified in the component description
             * \param bc - the {@link BundleContext} of the bundle containing the component
             * \param logger - the logger object used to log information from this class.
             *
             * \throws \c std::runtime_error if \c bc or \c logger is invalid
             */
            ReferenceManagerBaseImpl(metadata::ReferenceMetadata const& metadata,
                                     cppmicroservices::BundleContext const& bc,
                                     std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                     std::string const& configName);
            ReferenceManagerBaseImpl(ReferenceManagerBaseImpl const&) = delete;
            ReferenceManagerBaseImpl(ReferenceManagerBaseImpl&&) = delete;
            ReferenceManagerBaseImpl& operator=(ReferenceManagerBaseImpl const&) = delete;
            ReferenceManagerBaseImpl& operator=(ReferenceManagerBaseImpl&&) = delete;
            ~ReferenceManagerBaseImpl() override;

            /**
             * Returns name of the reference as specified in component description
             */
            std::string
            GetReferenceName() const override
            {
                return metadata_.name;
            }

            /**
             * Returns name of the reference as specified in component description
             */
            std::string
            GetReferenceScope() const override
            {
                return metadata_.scope;
            }

            /**
             * Returns \c LDAPString specifying the match criteria for this dependency
             */
            std::string
            GetLDAPString() const override
            {
                return metadata_.target;
            }

            /**
             * Returns \c true if the dependency is satisfied, \c false otherwise
             */
            bool IsSatisfied() const override;

            /**
             * Returns a set containing all {@link ServiceReferenceBase} objects that are
             * bound when the dependency criteria is satisfied
             */
            std::set<cppmicroservices::ServiceReferenceBase> GetBoundReferences() const override;

            /**
             * Returns a set containing all {@link ServiceReferenceBase} objects that match
             * the dependency criteria
             */
            std::set<cppmicroservices::ServiceReferenceBase> GetTargetReferences() const override;

            /**
             * Returns true if the cardinality for this reference is "optional"
             */
            bool IsOptional() const override;

            /**
             * Returns reference metadata associated with this reference
             */
            metadata::ReferenceMetadata const&
            GetMetadata() const
            {
                return metadata_;
            }

            /**
             * Implementation of the {@link ServiceTrackerCustomizer#AddingService} method.
             * The matched references and bound references are updated based on the
             * reference policy criteria
             *
             * \return A dummy object is returned from this method to the framework inorder to
             * receive the #RemovedService callback.
             */
            cppmicroservices::InterfaceMapConstPtr AddingService(
                cppmicroservices::ServiceReferenceU const& reference) override;

            /**
             * Implementation of the {@link ServiceTrackerCustomizer#ModifiedService} method.
             * No-op at this time
             */
            void ModifiedService(cppmicroservices::ServiceReferenceU const& reference,
                                 cppmicroservices::InterfaceMapConstPtr const& service) override;

            /**
             * Implementation of the {@link ServiceTrackerCustomizer#RemovedService} method.
             * The matched references and bound references are updated based on the
             * reference policy criteria
             */
            void RemovedService(cppmicroservices::ServiceReferenceU const& reference,
                                cppmicroservices::InterfaceMapConstPtr const& service) override;

            /**
             * Method is used to receive callbacks when the dependency is satisfied
             */
            cppmicroservices::ListenerTokenId RegisterListener(
                std::function<void(RefChangeNotification const&)> notify) override;

            /**
             * Method is used to remove the callbacks registered using RegisterListener
             */
            void UnregisterListener(cppmicroservices::ListenerTokenId token) override;

            /**
             * Method to stop tracking the reference service
             */
            void StopTracking() override;

            /**
             * Method returns true if maxCardinality is unary (1)
             */
            bool IsUnary() const override;

            /**
             * Method returns true if maxCardinality is multiple (n)
             */
            bool IsMultiple() const override;

            class BindingPolicy
            {
              public:
                virtual void ServiceAdded(ServiceReferenceBase const& reference) = 0;
                virtual void ServiceRemoved(ServiceReferenceBase const& reference) = 0;

                virtual ~BindingPolicy() = default;

              protected:
                // Some utility code to ensure we don't DRY too much in the subclasses. Code is
                // common code that was pulled up from the subclasses.
                void Log(std::string const& logStr,
                         cppmicroservices::logservice::SeverityLevel logLevel
                         = cppmicroservices::logservice::SeverityLevel::LOG_DEBUG);
                bool ShouldClearBoundRefs(ServiceReferenceBase const& reference);
                bool ShouldNotifySatisfied();
                void ClearBoundRefs();
                void StaticRemoveService(ServiceReferenceBase const& reference);
                void DynamicRemoveService(ServiceReferenceBase const& reference);

                explicit BindingPolicy(ReferenceManagerBaseImpl& parent) : mgr(parent) {}

                ReferenceManagerBaseImpl& mgr;

                BindingPolicy() = delete;
                BindingPolicy(BindingPolicy const&) = delete;
                BindingPolicy(BindingPolicy&&) = delete;
                BindingPolicy& operator=(BindingPolicy const&) = delete;
                BindingPolicy& operator=(BindingPolicy&&) = delete;
            };

            class BindingPolicyDynamicGreedy : public BindingPolicy
            {
              public:
                BindingPolicyDynamicGreedy(ReferenceManagerBaseImpl& parent) : BindingPolicy(parent) {}
                void ServiceAdded(ServiceReferenceBase const& reference) override;
                void ServiceRemoved(ServiceReferenceBase const& reference) override;
            };

            class BindingPolicyDynamicReluctant : public BindingPolicy
            {
              public:
                BindingPolicyDynamicReluctant(ReferenceManagerBaseImpl& parent) : BindingPolicy(parent) {}
                void ServiceAdded(ServiceReferenceBase const& reference) override;
                void ServiceRemoved(ServiceReferenceBase const& reference) override;
            };

            class BindingPolicyStaticGreedy : public BindingPolicy
            {
              public:
                BindingPolicyStaticGreedy(ReferenceManagerBaseImpl& parent) : BindingPolicy(parent) {}
                void ServiceAdded(ServiceReferenceBase const& reference) override;
                void ServiceRemoved(ServiceReferenceBase const& reference) override;
            };

            class BindingPolicyStaticReluctant : public BindingPolicy
            {
              public:
                BindingPolicyStaticReluctant(ReferenceManagerBaseImpl& parent) : BindingPolicy(parent) {}
                void ServiceAdded(ServiceReferenceBase const& reference) override;
                void ServiceRemoved(ServiceReferenceBase const& reference) override;
            };

            static std::unique_ptr<BindingPolicy> CreateBindingPolicy(ReferenceManagerBaseImpl& mgr,
                                                                      std::string const& policy,
                                                                      std::string const& policyOption);

            ReferenceManagerBaseImpl(metadata::ReferenceMetadata const& metadata,
                                     cppmicroservices::BundleContext const& bc,
                                     std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                     std::string const& configName,
                                     std::unique_ptr<BindingPolicy> policy);

          private:
            friend class ReferenceManagerImplTest;
            friend class BindingPolicyTest;

            static long GetServiceId(ServiceReferenceBase const& sRef);

            /**
             * Helper method to copy service references from #matchedRefs to #boundRefs
             * The copy is performed only if matchedRefs has sufficient items to
             * satisfy the reference's cardinality
             *
             * /return true on success, false otherwise.
             */
            bool UpdateBoundRefs();

            /**
             * Method used to send notifications to all the listeners
             */
            void BatchNotifyAllListeners(std::vector<RefChangeNotification> const& notification) noexcept;

            const metadata::ReferenceMetadata metadata_;    ///< reference information from the component description
            std::unique_ptr<ServiceTracker<void>> tracker; ///< used to track service availability
            std::shared_ptr<cppmicroservices::logservice::LogService> logger_; ///< logger for this runtime
            const std::string
                configName_; ///< Keep track of which component configuration object this reference manager belongs to.

            mutable cppmicroservices::Guarded<std::set<cppmicroservices::ServiceReferenceBase>>
                boundRefs; ///< guarded set of bound references
            mutable cppmicroservices::Guarded<std::set<cppmicroservices::ServiceReferenceBase>>
                matchedRefs; ///< guarded set of matched references

            mutable cppmicroservices::Guarded<RefMgrListenerMap> listenersMap;                    ///< guarded map of listeners
            static std::atomic<cppmicroservices::ListenerTokenId> tokenCounter; ///< used to
                                                                                /// generate unique
                                                                                /// tokens for
                                                                                /// listeners

            std::unique_ptr<BindingPolicy> bindingPolicy;
        };

        /**
         */
        class ReferenceManagerImpl final : public ReferenceManagerBaseImpl
        {
          public:
            ReferenceManagerImpl(metadata::ReferenceMetadata const& metadata,
                                 cppmicroservices::BundleContext const& bc,
                                 std::shared_ptr<cppmicroservices::logservice::LogService> logger,
                                 std::string const& configName)
                : ReferenceManagerBaseImpl(metadata, bc, logger, configName)
            {
            }
        };

    } // namespace scrimpl
} // namespace cppmicroservices
#endif // REFERENCEMANAGERIMPL_HPP
