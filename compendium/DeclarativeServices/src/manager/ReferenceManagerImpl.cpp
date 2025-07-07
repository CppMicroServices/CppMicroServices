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
#include <memory>

#include "ReferenceManagerImpl.hpp"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"

using cppmicroservices::Constants::SCOPE_PROTOTYPE;
using cppmicroservices::Constants::SERVICE_SCOPE;
using cppmicroservices::logservice::SeverityLevel;
using cppmicroservices::service::component::ComponentConstants::COMPONENT_NAME;
using cppmicroservices::service::component::ComponentConstants::REFERENCE_SCOPE_PROTOTYPE_REQUIRED;

namespace cppmicroservices
{
    namespace scrimpl
    {

        /**
         * @brief Returns the LDAPFilter of the reference metadata
         * @param refMetadata The metadata representing a service reference
         * @returns a LDAPFilter object corresponding to the @p refMetadata
         */
        LDAPFilter
        GetReferenceLDAPFilter(metadata::ReferenceMetadata const& refMetadata)
        {
            LDAPPropExpr expr;
            expr = (LDAPProp(cppmicroservices::Constants::OBJECTCLASS) == refMetadata.interfaceName);
            if (!refMetadata.target.empty())
            {
                expr &= LDAPPropExpr(refMetadata.target);
            }

            if (refMetadata.scope == REFERENCE_SCOPE_PROTOTYPE_REQUIRED)
            {
                expr &= (LDAPProp(SERVICE_SCOPE) == SCOPE_PROTOTYPE);
            }
            return LDAPFilter(expr);
        }

        ReferenceManagerBaseImpl::ReferenceManagerBaseImpl(
            metadata::ReferenceMetadata const& metadata,
            cppmicroservices::BundleContext const& bc,
            std::shared_ptr<cppmicroservices::logservice::LogService> logger,
            std::string const& configName)
            : ReferenceManagerBaseImpl(metadata,
                                       bc,
                                       logger,
                                       configName,
                                       CreateBindingPolicy(*this, metadata.getPolicy(), metadata.policyOption))
        {
        }

        ReferenceManagerBaseImpl::ReferenceManagerBaseImpl(
            metadata::ReferenceMetadata const& metadata,
            cppmicroservices::BundleContext const& bc,
            std::shared_ptr<cppmicroservices::logservice::LogService> logger,
            std::string const& configName,
            std::unique_ptr<BindingPolicy> policy)
            : metadata_(metadata)
            , tracker(nullptr)
            , logger_(std::move(logger))
            , configName_(configName)
            , bindingPolicy(std::move(policy))
        {
            if (!bc || !logger_)
            {
                throw std::invalid_argument("Failed to create object, Invalid arguments passed to constructor");
            }
            try
            {
                tracker = std::make_unique<ServiceTracker<void>>(bc, GetReferenceLDAPFilter(metadata_), this);
                tracker->Open();
            }
            catch (...)
            {
                logger_->Log(SeverityLevel::LOG_ERROR,
                            "could not open service tracker for " + metadata_.interfaceName,
                            std::current_exception());
                tracker.reset();
                throw std::current_exception();
            }
        }

        void
        ReferenceManagerBaseImpl::StopTracking()
        {
            try
            {
                tracker->Close();
            }
            catch (...)
            {
                logger_->Log(SeverityLevel::LOG_ERROR,
                            "Exception caught while closing service tracker for " + metadata_.interfaceName,
                            std::current_exception());
            }
        }

        std::set<cppmicroservices::ServiceReferenceBase>
        ReferenceManagerBaseImpl::GetBoundReferences() const
        {
            auto boundRefsHandle = boundRefs.lock();
            return std::set<cppmicroservices::ServiceReferenceBase>(boundRefsHandle->begin(), boundRefsHandle->end());
        }

        std::set<cppmicroservices::ServiceReferenceBase>
        ReferenceManagerBaseImpl::GetTargetReferences() const
        {
            auto matchedRefsHandle = matchedRefs.lock();
            return std::set<cppmicroservices::ServiceReferenceBase>(matchedRefsHandle->begin(),
                                                                    matchedRefsHandle->end());
        }

        bool
        ReferenceManagerBaseImpl::IsOptional() const
        {
            return (metadata_.minCardinality == 0);
        }

        bool
        ReferenceManagerBaseImpl::IsSatisfied() const
        {
            return (boundRefs.lock()->size() >= metadata_.minCardinality);
        }

        bool ReferenceManagerBaseImpl::IsUnary() const
        {
            return (metadata_.maxCardinality == 1);
        }

        bool ReferenceManagerBaseImpl::IsMultiple() const
        {
            return (metadata_.maxCardinality > 1);
        }

        ReferenceManagerBaseImpl::~ReferenceManagerBaseImpl() { StopTracking(); }

        struct dummyRefObj
        {
        };

        bool
        ReferenceManagerBaseImpl::UpdateBoundRefs()
        {
            auto matchedRefsHandle = matchedRefs.lock(); // acquires lock on matchedRefs
            auto const matchedRefsHandleSize = matchedRefsHandle->size();
            if (matchedRefsHandleSize >= metadata_.minCardinality)
            {
                auto boundRefsHandle = boundRefs.lock(); // acquires lock on boundRefs
                boundRefsHandle->clear();
                std::copy_n(matchedRefsHandle->rbegin(),
                            std::min(metadata_.maxCardinality, matchedRefsHandleSize),
                            std::inserter(*(boundRefsHandle), boundRefsHandle->begin()));
                return true;
            }
            return false;
            // release locks on matchedRefs and boundRefs
        }

        cppmicroservices::InterfaceMapConstPtr
        ReferenceManagerBaseImpl::AddingService(cppmicroservices::ServiceReference<void> const& reference)
        {
            // Each service registered by DS contains a service property representing the component configuration name
            // to which it belongs. By checking the component configuration name of a service it can be determined
            // whether this service will satisfy its own reference. If it would, it is not a matched reference as
            // a service from the same component configuration can't satisfy its own reference.
            //
            // ASSUMPTION: If there is no component configuration name then its assumed this service was not registered
            // by DS and could not satisfy itself since it is not managed by DS.
            auto const compConfigName = reference.GetProperty(COMPONENT_NAME);
            if ((true == compConfigName.Empty()) || (configName_ != compConfigName.ToStringNoExcept()))
            {
                // acquire lock on matchedRefs
                auto matchedRefsHandle = matchedRefs.lock();
                matchedRefsHandle->insert(reference);
            } // release lock on matchedRefs

            // After updating the bound references on this thread, notifying listeners happens on the same thread.
            // This behavior deviates from what is described in the "synchronous" section in
            // https://osgi.org/download/r6/osgi.core-6.0.0.pdf#page=432. Sporadically not returning a valid service
            // when a user calls getService, due to a service's references still resolving, was deemed undesirable
            // for user workflows.
            bindingPolicy->ServiceAdded(reference);

            // A non-null object must be returned to indicate to the ServiceTracker that
            // we are tracking the service and need to be called back when the service is removed.
            return MakeInterfaceMap<dummyRefObj>(std::make_shared<dummyRefObj>());
        }

        void
        ReferenceManagerBaseImpl::ModifiedService(cppmicroservices::ServiceReference<void> const& /*reference*/,
                                                  cppmicroservices::InterfaceMapConstPtr const& /*service*/)
        {
            // no-op since there is no use case for property update
        }

        /**
         * If a target service is available to replace the bound service which became unavailable,
         * the component configuration must be reactivated and the replacement service is bound to
         * the new component instance.
         */
        void
        ReferenceManagerBaseImpl::RemovedService(cppmicroservices::ServiceReference<void> const& reference,
                                                 cppmicroservices::InterfaceMapConstPtr const& /*service*/)
        {
            { // acquire lock on matchedRefs
                auto matchedRefsHandle = matchedRefs.lock();
                matchedRefsHandle->erase(reference);
            } // release lock on matchedRefs

            // After updating the bound references on this thread, notifying listeners happens on the same thread.
            // This behavior deviates from what is described in the "synchronous" section in
            // https://osgi.org/download/r6/osgi.core-6.0.0.pdf#page=432. Sometimes not returning a valid service
            // due to a service's references still resolving was deemed undesirable for user workflows.
            bindingPolicy->ServiceRemoved(reference);
        }

        std::atomic<cppmicroservices::ListenerTokenId> ReferenceManagerBaseImpl::tokenCounter(0);

        /**
         * Method is used to register a listener for callbacks
         */
        cppmicroservices::ListenerTokenId
        ReferenceManagerBaseImpl::RegisterListener(std::function<void(RefChangeNotification const&)> notify)
        {
            auto notifySatisfied = UpdateBoundRefs();
            if (notifySatisfied)
            {
                RefChangeNotification notification { metadata_.name, RefEvent::BECAME_SATISFIED };
                notify(notification);
            }

            cppmicroservices::ListenerTokenId retToken = ++tokenCounter;
            {
                auto listenerMapHandle = listenersMap.lock();
                listenerMapHandle->emplace(retToken, notify);
            }
            return retToken;
        }

        /**
         * Method is used to remove a registered listener
         */
        void
        ReferenceManagerBaseImpl::UnregisterListener(cppmicroservices::ListenerTokenId token)
        {
            auto listenerMapHandle = listenersMap.lock();
            listenerMapHandle->erase(token);
        }

        /**
         * Method used to notify all listeners
         */
        void
        ReferenceManagerBaseImpl::BatchNotifyAllListeners(
            std::vector<RefChangeNotification> const& notifications) noexcept
        {
            if (notifications.empty() || listenersMap.lock()->empty())
            {
                return;
            }

            RefMgrListenerMap listenersMapCopy;
            {
                auto listenerMapHandle = listenersMap.lock();
                listenersMapCopy = *listenerMapHandle; // copy the listeners map
            }

            for (auto& listenerPair : listenersMapCopy)
            {
                for (auto const& notification : notifications)
                {
                    try
                    {
                        listenerPair.second(notification);
                    }
                    catch (...)
                    {
                        logger_->Log(SeverityLevel::LOG_ERROR,
                                    "Exception caught while notifying service reference "
                                    "listeners for reference name "
                                        + notification.senderName,
                                    std::current_exception());
                    }
                }
            }
        }

        // util method to extract service-id from a given reference
        long
        ReferenceManagerBaseImpl::GetServiceId(ServiceReferenceBase const& sRef)
        {
            auto idAny = sRef.GetProperty(cppmicroservices::Constants::SERVICE_ID);
            return cppmicroservices::any_cast<long>(idAny);
        }

        std::unique_ptr<ReferenceManagerBaseImpl::BindingPolicy>
        ReferenceManagerBaseImpl::CreateBindingPolicy(ReferenceManagerBaseImpl& ref,
                                                      std::string const& policy,
                                                      std::string const& policyOption)
        {
            if (policy == "static")
            {
                if (policyOption == "reluctant")
                {
                    return std::make_unique<BindingPolicyStaticReluctant>(ref);
                }
                else
                { // greedy
                    return std::make_unique<BindingPolicyStaticGreedy>(ref);
                }
            }
            else
            { // dynamic
                if (policyOption == "reluctant")
                {
                    return std::make_unique<BindingPolicyDynamicReluctant>(ref);
                }
                else
                { // greedy
                    return std::make_unique<BindingPolicyDynamicGreedy>(ref);
                }
            }
        }

    } // namespace scrimpl
} // namespace cppmicroservices
