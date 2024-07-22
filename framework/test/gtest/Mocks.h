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

#ifndef CPPMICROSERVICES_FRAMEWORK_MOCKS_H
#define CPPMICROSERVICES_FRAMEWORK_MOCKS_H
#include "gmock/gmock.h"
#include "cppmicroservices/Any.h"
#include "cppmicroservices/AnyMap.h"
#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/BundleActivator.h"
#include "BundleArchive.h"
#include "cppmicroservices/BundleContext.h"
#include "BundleContextPrivate.h"
#include "cppmicroservices/BundleEvent.h"
#include "cppmicroservices/BundleEventHook.h"
#include "BundleEventInternal.h"
#include "cppmicroservices/BundleFindHook.h"
#include "BundleHooks.h"
#include "cppmicroservices/BundleImport.h"
#include "cppmicroservices/BundleInitialization.h"
#include "BundleManifest.h"
#include "BundlePrivate.h"
#include "BundleRegistry.h"
#include "cppmicroservices/BundleResource.h"
#include "BundleResourceContainer.h"
#include "cppmicroservices/BundleResourceStream.h"
#include "BundleStorage.h"
#include "BundleStorageFile.h"
#include "BundleStorageMemory.h"
#include "cppmicroservices/BundleTracker.h"
#include "cppmicroservices/BundleTrackerCustomizer.h"
#include "cppmicroservices/BundleVersion.h"
#include "CoreBundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"
#include "FrameworkPrivate.h"
#include "LDAPExpr.h"
#include "cppmicroservices/LDAPFilter.h"
#include "cppmicroservices/LDAPProp.h"
#include "cppmicroservices/ListenerFunctors.h"
#include "cppmicroservices/ListenerToken.h"
#include "Properties.h"
#include "cppmicroservices/PrototypeServiceFactory.h"
#include "Resolver.h"
#include "cppmicroservices/SecurityException.h"
#include "cppmicroservices/ServiceEvent.h"
#include "cppmicroservices/ServiceEventListenerHook.h"
#include "cppmicroservices/ServiceException.h"
#include "cppmicroservices/ServiceFactory.h"
#include "cppmicroservices/ServiceFindHook.h"
#include "ServiceHooks.h"
#include "cppmicroservices/ServiceInterface.h"
#include "ServiceListenerEntry.h"
#include "cppmicroservices/ServiceListenerHook.h"
#include "ServiceListenerHookPrivate.h"
#include "ServiceListeners.h"
#include "cppmicroservices/ServiceObjects.h"
#include "cppmicroservices/ServiceReference.h"
#include "cppmicroservices/ServiceReferenceBase.h"
#include "ServiceReferenceBasePrivate.h"
#include "cppmicroservices/ServiceRegistration.h"
#include "cppmicroservices/ServiceRegistrationBase.h"
#include "ServiceRegistrationCoreInfo.h"
#include "ServiceRegistrationLocks.h"
#include "ServiceRegistry.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "cppmicroservices/SharedLibrary.h"
#include "cppmicroservices/SharedLibraryException.h"
#include "cppmicroservices/ShrinkableMap.h"
#include "cppmicroservices/ShrinkableVector.h"
#include "Utils.h"
#include "cppmicroservices/detail/BundleAbstractTracked.h"
#include "cppmicroservices/detail/BundleResourceBuffer.h"
#include "cppmicroservices/detail/BundleTrackerPrivate.h"
#include "cppmicroservices/detail/CounterLatch.h"
#include "cppmicroservices/detail/ScopeGuard.h"
#include "cppmicroservices/detail/ServiceTrackerPrivate.h"
#include "cppmicroservices/detail/Threads.h"
#include "cppmicroservices/detail/TrackedBundle.h"
#include "cppmicroservices/detail/TrackedBundleListener.h"
#include "cppmicroservices/detail/TrackedService.h"
#include "cppmicroservices/detail/TrackedServiceListener.h"
#include "cppmicroservices/detail/WaitCondition.h"
#include "cppmicroservices/util/BundleObjFile.h"

using namespace cppmicroservices;

namespace cppmicroservices
{

    class MockAny : public cppmicroservices::Any
    {
      public:
        MockAny() : Any() {}
        MockAny(const Any & other) : Any(other) {}
        MockAny(Any && other) : Any(other) {}
        MOCK_METHOD1(Swap, Any &(Any &));
        MOCK_METHOD0(Empty, bool());
        MOCK_METHOD0(ToString, std::string());
        MOCK_METHOD0(ToStringNoExcept, std::string());
        MOCK_METHOD2(ToJSON, std::string(const uint8_t, const int32_t));
        MOCK_METHOD1(ToJSON, std::string(bool));
        MOCK_METHOD0(Type, const std::type_info &());
        MOCK_METHOD1(compare, bool(const Any &));
    };

    class MockAnyMap : public cppmicroservices::AnyMap
    {
      public:
        MockAnyMap(std::initializer_list<any_map::value_type> l) : AnyMap(l) {}
        MockAnyMap(map_type type, std::initializer_list<any_map::value_type> l) : AnyMap(type, l) {}
        MockAnyMap(const ordered_any_map & m) : AnyMap(m) {}
        MockAnyMap(ordered_any_map && m) : AnyMap(m) {}
        MockAnyMap(const unordered_any_map & m) : AnyMap(m) {}
        MockAnyMap(unordered_any_map && m) : AnyMap(m) {}
        MockAnyMap(const unordered_any_cimap & m) : AnyMap(m) {}
        MockAnyMap(unordered_any_cimap && m) : AnyMap(m) {}
        MOCK_METHOD0(GetType, map_type());
        MOCK_METHOD1(AtCompoundKey, const mapped_type &(const key_type &));
        MOCK_METHOD2(AtCompoundKey, mapped_type(const key_type &, mapped_type));
    };

    template <class T>
    class MockAtomic : public cppmicroservices::detail::Atomic<T>
    {
      public:
        MockAtomic() : cppmicroservices::detail::Atomic<T>() {}
        MOCK_METHOD0(Load, T());
        MOCK_METHOD1(Store, void(const T &));
        MOCK_METHOD1(Exchange, T(const T &));
        MOCK_METHOD2(CompareExchange, bool(T &, const T &));
    };

    class MockBadAnyCastException : public cppmicroservices::BadAnyCastException
    {
      public:
        MockBadAnyCastException(std::string msg) : BadAnyCastException(msg) {}
    };

    class MockBundle : public cppmicroservices::Bundle
    {
      public:
        MockBundle(const std::shared_ptr<BundlePrivate> & d) : Bundle(d) {}
        MockBundle(const Bundle &) : Bundle() {}
        MockBundle(Bundle &&) : Bundle() {}
        MockBundle() : Bundle() {}
        MOCK_CONST_METHOD0(GetState, State());
        MOCK_CONST_METHOD0(GetBundleContext, BundleContext());
        MOCK_CONST_METHOD0(GetBundleId, long());
        MOCK_CONST_METHOD0(GetLocation, std::string());
        MOCK_CONST_METHOD2(GetSymbol, void *(void *, const std::string &));
        MOCK_CONST_METHOD0(GetSymbolicName, std::string());
        MOCK_CONST_METHOD0(GetVersion, BundleVersion());
        MOCK_CONST_METHOD0(GetProperties, std::map<std::string, Any>());
        MOCK_CONST_METHOD0(GetHeaders, const AnyMap &());
        MOCK_CONST_METHOD1(GetProperty, Any(const std::string &));
        MOCK_CONST_METHOD0(GetPropertyKeys, std::vector<std::string>());
        MOCK_CONST_METHOD0(GetRegisteredServices, std::vector<ServiceReferenceU>());
        MOCK_CONST_METHOD0(GetServicesInUse, std::vector<ServiceReferenceU>());
        MOCK_CONST_METHOD1(GetResource, BundleResource(const std::string &));
        MOCK_CONST_METHOD3(FindResources, std::vector<BundleResource>(const std::string &, const std::string &, bool));
        MOCK_CONST_METHOD0(GetLastModified, TimeStamp());
        MOCK_METHOD1(Start, void(uint32_t));
        MOCK_METHOD0(Start, void());
        MOCK_METHOD1(Stop, void(uint32_t));
        MOCK_METHOD0(Stop, void());
        MOCK_METHOD0(Uninstall, void());
    };

    template <class S, class T, class R>
    class MockBundleAbstractTracked : public cppmicroservices::detail::BundleAbstractTracked<S, T, R>
    {
      using TrackingMap = typename std::unordered_map<Bundle, T>;
      public:
        MockBundleAbstractTracked<S, T, R>(BundleContext bc) : cppmicroservices::detail::BundleAbstractTracked<S, T, R>(bc) {}
        MOCK_METHOD1(SetInitial, void(const std::vector<S> &));
        MOCK_METHOD0(TrackInitial, void());
        MOCK_METHOD0(Close, void());
        MOCK_METHOD2(Track, void(S, R));
        MOCK_METHOD2(Untrack, void(S, R));
        MOCK_METHOD0(Size_unlocked, std::size_t());
        MOCK_METHOD0(IsEmpty_unlocked, bool());
        MOCK_METHOD1(GetCustomizedObject_unlocked, std::optional<T>(S));
        MOCK_METHOD1(GetTracked_unlocked, void(std::vector<S> &));
        MOCK_METHOD0(Modified, void());
        MOCK_METHOD0(GetTrackingCount, int());
        MOCK_METHOD1(CopyEntries_unlocked, void(TrackingMap &));
        MOCK_METHOD2(CustomizerAdding, std::optional<T>(S, const R &));
        MOCK_METHOD3(CustomizerModified, void(S, const R &, const T &));
        MOCK_METHOD3(CustomizerRemoved, void(S, const R &, const T &));
        MOCK_METHOD2(TrackAdding, void(S, R));
        MOCK_METHOD2(CustomizerAddingFinal, bool(S, const std::optional<T> &));
    };

    class MockBundleActivator : public cppmicroservices::BundleActivator
    {
      public:
        MOCK_METHOD1(Start, void(BundleContext));
        MOCK_METHOD1(Stop, void(BundleContext));
    };

    class MockBundleArchive : public cppmicroservices::BundleArchive
    {
      public:
        MockBundleArchive(const BundleArchive &) : BundleArchive() {}
        MockBundleArchive() : BundleArchive() {}
        MockBundleArchive(BundleStorage * storage, std::shared_ptr<BundleResourceContainer> resourceContainer, std::string resourcePrefix, std::string location, long bundleId, AnyMap bundleManifest) : BundleArchive(storage, resourceContainer, resourcePrefix, location, bundleId, bundleManifest) {}
        MOCK_METHOD0(IsValid, bool());
        MOCK_METHOD0(Purge, void());
        MOCK_METHOD0(GetBundleId, long());
        MOCK_METHOD0(GetBundleLocation, std::string());
        MOCK_METHOD0(GetResourcePrefix, std::string());
        MOCK_METHOD1(GetResource, BundleResource(const std::string &));
        MOCK_METHOD3(FindResources, std::vector<BundleResource>(const std::string &, const std::string &, bool));
        MOCK_METHOD0(GetLastModified, TimeStamp());
        MOCK_METHOD1(SetLastModified, void(const TimeStamp &));
        MOCK_METHOD0(GetAutostartSetting, int32_t());
        MOCK_METHOD1(SetAutostartSetting, void(int32_t));
        MOCK_METHOD0(GetResourceContainer, std::shared_ptr<BundleResourceContainer>());
        MOCK_METHOD0(GetInjectedManifest, const AnyMap &());
    };

    template <class I1, class... Interfaces>
    class MockBundleContext : public cppmicroservices::BundleContext
    {
      public:
        MockBundleContext() : BundleContext() {}
        MockBundleContext(std::shared_ptr<BundleContextPrivate> ctx) : BundleContext(ctx) {}
        MOCK_METHOD1(GetProperty, Any(const std::string &));
        MOCK_METHOD0(GetProperties, AnyMap());
        MOCK_METHOD0(GetBundle, Bundle());
        MOCK_METHOD1(GetBundle, Bundle(long));
        MOCK_METHOD1(GetBundles, std::vector<Bundle>(const std::string &));
        MOCK_METHOD0(GetBundles, std::vector<Bundle>());
        MOCK_METHOD2(RegisterService, ServiceRegistrationU(const InterfaceMapConstPtr &, const ServiceProperties &));
        MOCK_METHOD0(RegisterService, ServiceRegistration<I1, Interfaces...>());
        MOCK_METHOD2(GetServiceReferences, std::vector<ServiceReferenceU>(const std::string &, const std::string &));
        MOCK_METHOD1(GetServiceReference, ServiceReferenceU(const std::string &));
        MOCK_METHOD1(GetService, std::shared_ptr<void>(const ServiceReferenceBase &));
        MOCK_METHOD1(GetService, InterfaceMapConstPtr(const ServiceReferenceU &));
        MOCK_METHOD2(AddServiceListener, ListenerToken(const ServiceListener &, const std::string &));
        MOCK_METHOD1(RemoveServiceListener, void(const ServiceListener &));
        MOCK_METHOD1(AddBundleListener, ListenerToken(const BundleListener &));
        MOCK_METHOD1(RemoveBundleListener, void(const BundleListener &));
        MOCK_METHOD1(AddFrameworkListener, ListenerToken(const FrameworkListener &));
        MOCK_METHOD1(RemoveFrameworkListener, void(const FrameworkListener &));
        MOCK_METHOD1(RemoveListener, void(ListenerToken));
        MOCK_METHOD0(AddServiceListener, ListenerToken());
        MOCK_METHOD0(RemoveServiceListener, void());
        MOCK_METHOD0(AddBundleListener, ListenerToken());
        MOCK_METHOD0(RemoveBundleListener, void());
        MOCK_METHOD0(AddFrameworkListener, ListenerToken());
        MOCK_METHOD0(RemoveFrameworkListener, void());
        MOCK_METHOD1(GetDataFile, std::string(const std::string &));
        MOCK_METHOD2(InstallBundles, std::vector<Bundle>(const std::string &, const cppmicroservices::AnyMap &));
        MOCK_METHOD1(MakeBundleContext, BundleContext(BundleContextPrivate *));
        MOCK_METHOD1(MakeBundleContext, BundleContext(const std::shared_ptr<BundleContextPrivate> &));
        MOCK_METHOD1(GetPrivate, std::shared_ptr<BundleContextPrivate>(const BundleContext &));
        MOCK_METHOD3(AddServiceListener, ListenerToken(const ServiceListener &, void *, const std::string &));
        MOCK_METHOD2(RemoveServiceListener, void(const ServiceListener &, void *));
        MOCK_METHOD2(AddBundleListener, ListenerToken(const BundleListener &, void *));
        MOCK_METHOD2(RemoveBundleListener, void(const BundleListener &, void *));
    };

    class MockBundleContextPrivate : public cppmicroservices::BundleContextPrivate
    {
      public:
        MockBundleContextPrivate(BundlePrivate * bundle) : BundleContextPrivate(bundle) {}
        MOCK_METHOD0(IsValid, bool());
        MOCK_METHOD0(CheckValid, void());
        MOCK_METHOD0(Invalidate, void());
    };

    class MockBundleEvent : public cppmicroservices::BundleEvent
    {
      public:
        MockBundleEvent() : BundleEvent() {}
        MockBundleEvent(Type type, const Bundle & bundle) : BundleEvent(type, bundle) {}
        MockBundleEvent(Type type, const Bundle & bundle, const Bundle & origin) : BundleEvent(type, bundle, origin) {}
        MOCK_METHOD0(GetBundle, Bundle());
        MOCK_METHOD0(GetType, Type());
        MOCK_METHOD0(GetOrigin, Bundle());
    };

    class MockBundleEventHook : public cppmicroservices::BundleEventHook
    {
      public:
        MOCK_METHOD2(Event, void(const BundleEvent &, ShrinkableVector<BundleContext> &));
    };

    class MockBundleEventInternal : public cppmicroservices::BundleEventInternal
    {
      public:
        MockBundleEventInternal(BundleEvent::Type t, const std::shared_ptr<BundlePrivate> & b) : BundleEventInternal(t, b) {}
    };

    class MockBundleFindHook : public cppmicroservices::BundleFindHook
    {
      public:
        MOCK_METHOD2(Find, void(const BundleContext &, ShrinkableVector<Bundle> &));
    };

    class MockBundleHooks : public cppmicroservices::BundleHooks
    {
      public:
        MockBundleHooks(CoreBundleContext * ctx) : BundleHooks(ctx) {}
        MOCK_METHOD2(FilterBundle, Bundle(const BundleContext &, const Bundle &));
        MOCK_METHOD2(FilterBundles, void(const BundleContext &, std::vector<Bundle> &));
        MOCK_METHOD2(FilterBundleEventReceivers, void(const BundleEvent &, ServiceListeners::BundleListenerMap &));
    };

    class MockBundleManifest : public cppmicroservices::BundleManifest
    {
      public:
        MockBundleManifest() : BundleManifest() {}
        MockBundleManifest(const AnyMap & m) : BundleManifest(m) {}
        MOCK_METHOD1(Parse, void(std::istream &));
        MOCK_CONST_METHOD0(GetHeaders, const AnyMap &());
        MOCK_METHOD1(Contains, bool(const std::string &));
        MOCK_METHOD1(GetValue, Any(const std::string &));
        MOCK_METHOD1(GetValueDeprecated, Any(const std::string &));
        MOCK_METHOD0(GetKeysDeprecated, std::vector<std::string>());
        MOCK_METHOD0(GetPropertiesDeprecated, std::map<std::string, Any>());
        MOCK_METHOD0(CopyDeprecatedProperties, void());
    };

    class MockBundlePrivate : public cppmicroservices::BundlePrivate
    {
      public:
        MockBundlePrivate(CoreBundleContext * coreCtx) : BundlePrivate(coreCtx) {}
        MockBundlePrivate(CoreBundleContext * coreCtx, const std::shared_ptr<BundleArchive> & ba) : BundlePrivate(coreCtx, ba) {}
        MOCK_METHOD0(CheckUninstalled, void());
        MOCK_METHOD0(RemoveBundleResources, void());
        MOCK_METHOD1(Stop, void(uint32_t));
        MOCK_METHOD0(Stop0, std::exception_ptr());
        MOCK_METHOD0(Stop1, std::exception_ptr());
        MOCK_METHOD0(Stop2, void());
        MOCK_CONST_METHOD0(GetUpdatedState, Bundle::State());
        MOCK_METHOD1(SetStateInstalled, void(bool));
        MOCK_METHOD0(Purge, void());
        MOCK_CONST_METHOD0(GetBundleArchive, std::shared_ptr<BundleArchive>());
        MOCK_METHOD1(SetAutostartSetting, void(int32_t));
        MOCK_CONST_METHOD0(GetAutostartSetting, int32_t());
        MOCK_METHOD0(FinalizeActivation, void());
        MOCK_METHOD0(Uninstall, void());
        MOCK_CONST_METHOD0(GetLocation, std::string());
        MOCK_METHOD1(Start, void(uint32_t));
        MOCK_CONST_METHOD0(GetHeaders, const AnyMap &());
        MOCK_METHOD0(Start0, std::exception_ptr());
        MOCK_METHOD0(StartFailed, void());
    };

    class MockBundleRegistry : public cppmicroservices::BundleRegistry
    {
      public:
        using BundleMap = std::multimap<std::string, std::shared_ptr<BundlePrivate>>;
        MockBundleRegistry() : BundleRegistry(nullptr) {}
        MockBundleRegistry(CoreBundleContext * coreCtx) : BundleRegistry(coreCtx) {}
        MOCK_METHOD0(Init, void());
        MOCK_METHOD0(Clear, void());
        MOCK_METHOD3(Install, std::vector<Bundle>(const std::string &, BundlePrivate *, const cppmicroservices::AnyMap &));
        MOCK_METHOD2(Remove, void(const std::string &, long));
        MOCK_METHOD1(GetBundle, std::shared_ptr<BundlePrivate>(long));
        MOCK_METHOD1(GetBundles, std::vector<std::shared_ptr<BundlePrivate>>(const std::string &));
        MOCK_METHOD2(GetBundles, std::vector<std::shared_ptr<BundlePrivate>>(const std::string &, const BundleVersion &));
        MOCK_METHOD0(GetBundles, std::vector<std::shared_ptr<BundlePrivate>>());
        MOCK_METHOD0(GetActiveBundles, std::vector<std::shared_ptr<BundlePrivate>>());
        MOCK_METHOD0(Load, void());
        MOCK_METHOD4(Install0, std::vector<Bundle>(const std::string &, const std::shared_ptr<BundleResourceContainer> &, const std::vector<std::string> &, const cppmicroservices::AnyMap &));
        MOCK_METHOD0(CheckIllegalState, void());
        MOCK_METHOD5(GetAlreadyInstalledBundlesAtLocation, std::shared_ptr<BundleResourceContainer>(std::pair<BundleMap::iterator, BundleMap::iterator>, const std::string &, const cppmicroservices::AnyMap &, std::vector<Bundle> &, std::vector<std::string> &));
        MOCK_METHOD2(DecrementInitialBundleMapRef, void(cppmicroservices::detail::MutexLockingStrategy<>::UniqueLock &, const std::string &));
    };

    class MockBundleResource : public cppmicroservices::BundleResource
    {
      public:
        MockBundleResource() : BundleResource() {}
        MockBundleResource(const BundleResource & resource) : BundleResource(resource) {}
        MOCK_METHOD0(IsValid, bool());
        MOCK_METHOD0(GetName, std::string());
        MOCK_METHOD0(GetPath, std::string());
        MOCK_METHOD0(GetResourcePath, std::string());
        MOCK_METHOD0(GetBaseName, std::string());
        MOCK_METHOD0(GetCompleteBaseName, std::string());
        MOCK_METHOD0(GetSuffix, std::string());
        MOCK_METHOD0(GetCompleteSuffix, std::string());
        MOCK_METHOD0(IsDir, bool());
        MOCK_METHOD0(IsFile, bool());
        MOCK_METHOD0(GetChildren, std::vector<std::string>());
        MOCK_METHOD0(GetChildResources, std::vector<BundleResource>());
        MOCK_METHOD0(GetSize, int());
        MOCK_METHOD0(GetCompressedSize, int());
        MOCK_METHOD0(GetLastModified, time_t());
        MOCK_METHOD0(GetCrc32, uint32_t());
        MOCK_METHOD0(InitializeChildren, void());
        MOCK_METHOD0(Hash, std::size_t());
        MOCK_METHOD0(GetData, std::unique_ptr<void, void (*)(void *)>());
    };

    class MockBundleResourceBuffer : public cppmicroservices::detail::BundleResourceBuffer
    {
      public:
        MOCK_METHOD0(underflow, int_type());
        MOCK_METHOD0(uflow, int_type());
        MOCK_METHOD1(pbackfail, int_type(int_type));
        MOCK_METHOD0(showmanyc, std::streamsize());
        MOCK_METHOD3(seekoff, pos_type(off_type, std::ios_base::seekdir, std::ios_base::openmode));
        MOCK_METHOD2(seekpos, pos_type(pos_type, std::ios_base::openmode));
    };

    class MockBundleResourceContainer : public cppmicroservices::BundleResourceContainer
    {
        using cppmicroservices::BundleResourceContainer::GetTopLevelDirs;
      public:
        MOCK_METHOD0(GetLocation, std::string());
        MOCK_METHOD0(GetTopLevelDirs, std::vector<std::string>());
        MOCK_METHOD1(GetStat, bool(Stat &));
        MOCK_METHOD2(GetStat, bool(int, Stat &));
        MOCK_METHOD1(GetData, std::unique_ptr<void, void (*)(void *)>(int));
        MOCK_METHOD4(GetChildren, void(const std::string &, bool, std::vector<std::string> &, std::vector<uint32_t> &));
        MOCK_METHOD5(FindNodes, void(const std::shared_ptr<const BundleArchive> &, const std::string &, const std::string &, bool, std::vector<BundleResource> &));
        MOCK_METHOD0(CloseContainer, void());
        MOCK_METHOD0(InitSortedEntries, void());
        MOCK_METHOD2(Matches, bool(const std::string &, const std::string &));
        MOCK_METHOD0(InitMiniz, void());
        MOCK_METHOD0(OpenAndInitializeContainer, void());
    };

    class MockBundleResourceStream : public cppmicroservices::BundleResourceStream
    {
      public:
        MockBundleResourceStream(const BundleResource & resource, std::ios_base::openmode mode) : BundleResourceStream(resource, mode) {}
    };

    class MockBundleStorageFile : public cppmicroservices::BundleStorageFile
    {
      public:
        MockBundleStorageFile() : BundleStorageFile() {}
        MOCK_METHOD3(CreateAndInsertArchive, std::shared_ptr<BundleArchive>(const std::shared_ptr<BundleResourceContainer> &, const std::string &, const ManifestT &));
        MOCK_METHOD1(RemoveArchive, bool(const BundleArchive *));
        // TODO: MOCK_METHOD0(GetAllBundleArchives, std::vector<std::shared_ptr<BundleArchive>>());
        // TODO: MOCK_METHOD0(GetStartOnLaunchBundles, std::vector<long>());
        MOCK_METHOD0(Close, void());
    };

    class MockBundleStorageMemory : public cppmicroservices::BundleStorageMemory
    {
      public:
        MockBundleStorageMemory() : BundleStorageMemory() {}
        ~MockBundleStorageMemory() = default;
        MOCK_METHOD3(CreateAndInsertArchive, std::shared_ptr<BundleArchive>(const std::shared_ptr<BundleResourceContainer> &, const std::string &, const ManifestT &));
        MOCK_METHOD1(RemoveArchive, bool(const BundleArchive *));
        // TODO: MOCK_METHOD0(GetAllBundleArchives, std::vector<std::shared_ptr<BundleArchive>>());
        // TODO: MOCK_METHOD0(GetStartOnLaunchBundles, std::vector<long>());
        MOCK_METHOD0(Close, void());
    };

    template <class T = Bundle>
    class MockBundleTracker : public cppmicroservices::BundleTracker<T>
    {
      public:
        using TrackingMap = typename std::unordered_map<Bundle, T>;
        using BundleStateMaskType = std::underlying_type_t<Bundle::State>;
        MOCK_METHOD0(CreateStateMask, typename BundleTracker<T>::BundleStateMaskType());
        MockBundleTracker(const BundleContext & context, const BundleStateMaskType stateMask, const std::shared_ptr<BundleTrackerCustomizer<T>> customizer) : BundleTracker<T>(context, stateMask, customizer) {}
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(GetBundles, std::vector<Bundle>());
        MOCK_METHOD1(GetObject, std::optional<T>(const Bundle &));
        MOCK_METHOD0(GetTracked, TrackingMap());
        MOCK_METHOD0(GetTrackingCount, int());
        MOCK_METHOD0(IsEmpty, bool());
        MOCK_METHOD0(Open, void());
        MOCK_METHOD1(Remove, void(const Bundle &));
        MOCK_METHOD0(Size, size_t());
        MOCK_METHOD2(AddingBundle, std::optional<T>(const Bundle &, const BundleEvent &));
        MOCK_METHOD3(ModifiedBundle, void(const Bundle &, const BundleEvent &, const T &));
        MOCK_METHOD3(RemovedBundle, void(const Bundle &, const BundleEvent &, const T &));
    };

    template <class T>
    class MockBundleTrackerCustomizer : public cppmicroservices::BundleTrackerCustomizer<T>
    {
      public:
        MOCK_METHOD1(ConvertToTrackedType, T(const Bundle &));
        MOCK_METHOD2(AddingBundle, std::optional<T>(const Bundle &, const BundleEvent &));
        MOCK_METHOD3(ModifiedBundle, void(const Bundle &, const BundleEvent &, const T &));
        MOCK_METHOD3(RemovedBundle, void(const Bundle &, const BundleEvent &, const T &));
    };

    template <class T>
    class MockBundleTrackerPrivate : public cppmicroservices::detail::BundleTrackerPrivate<T>
    {
      public:
        using BundleStateMaskType = std::underlying_type_t<Bundle::State>;
        MockBundleTrackerPrivate(BundleTracker<T> * bundleTracker, const BundleContext & context, const BundleStateMaskType stateMask, const std::shared_ptr<BundleTrackerCustomizer<T>> customizer) : cppmicroservices::detail::BundleTrackerPrivate<T>(bundleTracker, context, stateMask, customizer) {}
        MOCK_METHOD1(GetInitialBundles, std::vector<Bundle>(BundleStateMaskType));
        MOCK_METHOD2(GetBundles_unlocked, void(std::vector<Bundle> &, cppmicroservices::detail::TrackedBundle<T> *));
        MOCK_METHOD0(Tracked, std::shared_ptr<cppmicroservices::detail::TrackedBundle<T>>());
        MOCK_METHOD0(Modified, void());
        MOCK_METHOD0(GetCustomizer_unlocked, BundleTrackerCustomizer<T> *());
    };

    class MockBundleVersion : public cppmicroservices::BundleVersion
    {
      public:
        MOCK_METHOD0(Validate, void());
        MOCK_METHOD0(EmptyVersion, BundleVersion());
        MOCK_METHOD0(UndefinedVersion, BundleVersion());
        MockBundleVersion(unsigned int majorVersion, unsigned int minorVersion, unsigned int microVersion) : BundleVersion(majorVersion, minorVersion, microVersion) {}
        MockBundleVersion(unsigned int majorVersion, unsigned int minorVersion, unsigned int microVersion, std::string qualifier) : BundleVersion(majorVersion, minorVersion, microVersion, qualifier) {}
        MockBundleVersion(const std::string & version) : BundleVersion(version) {}
        MockBundleVersion(const BundleVersion & version) : BundleVersion(version) {}
        MOCK_METHOD1(ParseVersion, BundleVersion(const std::string &));
        MOCK_METHOD0(IsUndefined, bool());
        MOCK_METHOD0(GetMajor, unsigned int());
        MOCK_METHOD0(GetMinor, unsigned int());
        MOCK_METHOD0(GetMicro, unsigned int());
        MOCK_METHOD0(GetQualifier, std::string());
        MOCK_METHOD0(ToString, std::string());
        MOCK_METHOD1(Compare, int(const BundleVersion &));
    };

    class MockCoreBundleContext : public cppmicroservices::CoreBundleContext
    {
      public:
        MockCoreBundleContext() : CoreBundleContext({}, nullptr) {}
        MOCK_METHOD1(SetThis, void(const std::shared_ptr<CoreBundleContext> &));
        MOCK_METHOD0(Init, void());
        MOCK_METHOD0(Uninit0, void());
        MOCK_METHOD0(Uninit1, void());
        MOCK_METHOD1(SetFrameworkStateAndBlockUntilComplete, WriteLock(bool));
        MOCK_METHOD0(GetFrameworkStateAndBlock, std::unique_ptr<FrameworkShutdownBlocker>());
        MOCK_METHOD1(GetDataStorage, std::string(long));
    };

    class MockFramework : public cppmicroservices::Framework
    {
      public:
        MockFramework(Bundle b) : Framework(b) {}
        MockFramework(const Framework & fw) : Framework(fw) {}
        MockFramework(Framework && fw) : Framework(fw) {}
        MOCK_METHOD0(Init, void());
        MOCK_METHOD1(WaitForStop, FrameworkEvent(const std::chrono::milliseconds &));
    };

    class MockFrameworkEvent : public cppmicroservices::FrameworkEvent
    {
      public:
        MockFrameworkEvent() : FrameworkEvent() {}
        MockFrameworkEvent(Type type, const Bundle & bundle, const std::string & message, const std::exception_ptr exception) : FrameworkEvent(type, bundle, message, exception) {}
        MOCK_METHOD0(GetBundle, Bundle());
        MOCK_METHOD0(GetMessage, std::string());
        MOCK_METHOD0(GetThrowable, std::exception_ptr());
        MOCK_METHOD0(GetType, Type());
    };

    class MockFrameworkFactory : public cppmicroservices::FrameworkFactory
    {
      public:
        MOCK_METHOD2(NewFramework, Framework(const FrameworkConfiguration &, std::ostream *));
        MOCK_METHOD0(NewFramework, Framework());
        MOCK_METHOD2(NewFramework, Framework(const std::map<std::string, Any> &, std::ostream *));
    };

    class MockFrameworkPrivate : public cppmicroservices::FrameworkPrivate
    {
      public:
        MockFrameworkPrivate(CoreBundleContext * fwCtx) : FrameworkPrivate(fwCtx) {}
        MOCK_METHOD0(Init, void());
        MOCK_METHOD0(DoInit, void());
        MOCK_METHOD0(InitSystemBundle, void());
        MOCK_METHOD0(UninitSystemBundle, void());
        MOCK_METHOD1(WaitForStop, FrameworkEvent(const std::chrono::milliseconds &));
        MOCK_METHOD1(Shutdown, void(bool));
        MOCK_METHOD1(Start, void(uint32_t));
        MOCK_METHOD1(Stop, void(uint32_t));
        MOCK_METHOD0(Uninstall, void());
        MOCK_METHOD2(Shutdown0, void(bool, bool));
        MOCK_METHOD1(ShutdownDone_unlocked, void(bool));
        MOCK_METHOD0(StopAllBundles, void());
        MOCK_METHOD1(SystemShuttingdownDone_unlocked, void(const FrameworkEventInternal &));
    };

    template <class Interfaces, size_t size>
    class MockInsertInterfaceHelper : public cppmicroservices::detail::InsertInterfaceHelper<Interfaces, size>
    {
      public:
        MOCK_METHOD2(insert, void(InterfaceMapPtr &, const Interfaces &));
    };

    template <template <class...> class List, class... Args>
    class MockInterfacesTuple : public cppmicroservices::detail::InterfacesTuple<List, Args...>
    {
      public:
        using type = List<std::shared_ptr<Args>...>;
        MOCK_METHOD0(create, type());
    };

    class MockListenerToken : public cppmicroservices::ListenerToken
    {
      public:
        MockListenerToken() : ListenerToken() {}
        MOCK_METHOD0(Id, ListenerTokenId());
    };

    template <class Mutex = std::mutex>
    class MockMutexLockingStrategy : public cppmicroservices::detail::MutexLockingStrategy<Mutex>
    {
      public:
        MockMutexLockingStrategy() : cppmicroservices::detail::MutexLockingStrategy<Mutex>() {}
        MOCK_METHOD0(Lock, void());
        MOCK_METHOD0(UnLock, void());
        MOCK_METHOD0(TryLockFor, bool());
    };

    class MockProperties : public cppmicroservices::Properties
    {
      public:
        MockProperties(const AnyMap & props) : Properties(props) {}
        MockProperties(AnyMap && props) : Properties(props) {}
        MOCK_METHOD2(ValueByRef_unlocked, const Any &(const std::string &, bool));
        MOCK_METHOD2(Value_unlocked, std::pair<Any, bool>(const std::string &, bool));
        MOCK_METHOD0(Keys_unlocked, std::vector<std::string>());
        MOCK_METHOD0(Clear_unlocked, void());
        MOCK_METHOD0(GetPropsAnyMap, const AnyMap &());
        MOCK_METHOD0(PopulateCaseInsensitiveLookupMap, void());
    };

    class MockPropertiesHandle : public cppmicroservices::PropertiesHandle
    {
      public:
        MockPropertiesHandle(const Properties & props, bool lock) : PropertiesHandle(props, lock) {}
    };

    class MockPrototypeServiceFactory : public cppmicroservices::PrototypeServiceFactory
    {
      public:
        MOCK_METHOD2(GetService, InterfaceMapConstPtr(const Bundle &, const ServiceRegistrationBase &));
        MOCK_METHOD3(UngetService, void(const Bundle &, const ServiceRegistrationBase &, const InterfaceMapConstPtr &));
    };

    class MockRawBundleResources : public cppmicroservices::RawBundleResources
    {
      public:
        MOCK_METHOD0(GetData, void *());
        MOCK_METHOD0(GetSize, std::size_t());
    };

    class MockServiceEvent : public cppmicroservices::ServiceEvent
    {
      public:
        MockServiceEvent() : ServiceEvent() {}
        MockServiceEvent(Type type, const ServiceReferenceBase & reference) : ServiceEvent(type, reference) {}
        MockServiceEvent(const ServiceEvent & other) : ServiceEvent(other) {}
        MOCK_METHOD0(GetType, Type());
    };

    class MockServiceEventListenerHook : public cppmicroservices::ServiceEventListenerHook
    {
      public:
        MOCK_METHOD2(Event, void(const ServiceEvent &, ShrinkableMapType &));
    };

    class MockServiceException : public cppmicroservices::ServiceException
    {
      public:
        MockServiceException(const std::string & msg, const Type & type) : ServiceException(msg, type) {}
        MockServiceException(const ServiceException & o) : ServiceException(o) {}
        MOCK_METHOD0(GetType, Type());
    };

    class MockServiceFactory : public cppmicroservices::ServiceFactory
    {
      public:
        MOCK_METHOD2(GetService, InterfaceMapConstPtr(const Bundle &, const ServiceRegistrationBase &));
        MOCK_METHOD3(UngetService, void(const Bundle &, const ServiceRegistrationBase &, const InterfaceMapConstPtr &));
    };

    class MockServiceFindHook : public cppmicroservices::ServiceFindHook
    {
      public:
        MOCK_METHOD4(Find, void(const BundleContext &, const std::string &, const std::string &, ShrinkableVector<ServiceReferenceBase> &));
    };

    class MockServiceHooks : public cppmicroservices::ServiceHooks
    {
      public:
        MOCK_METHOD1(AddingService, std::shared_ptr<ServiceListenerHook>(const ServiceReference<ServiceListenerHook> &));
        MOCK_METHOD2(ModifiedService, void(const ServiceReference<ServiceListenerHook> &, const std::shared_ptr<ServiceListenerHook> &));
        MOCK_METHOD2(RemovedService, void(const ServiceReference<ServiceListenerHook> &, const std::shared_ptr<ServiceListenerHook> &));
        MockServiceHooks(CoreBundleContext * coreCtx) : ServiceHooks(coreCtx) {}
        MOCK_METHOD0(Open, void());
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(IsOpen, bool());
        MOCK_METHOD4(FilterServiceReferences, void(BundleContextPrivate *, const std::string &, const std::string &, std::vector<ServiceReferenceBase> &));
        MOCK_METHOD2(FilterServiceEventReceivers, void(const ServiceEvent &, ServiceListeners::ServiceListenerEntries &));
        MOCK_METHOD1(HandleServiceListenerReg, void(const ServiceListenerEntry &));
        MOCK_METHOD1(HandleServiceListenerUnreg, void(const ServiceListenerEntry &));
        MOCK_METHOD1(HandleServiceListenerUnreg, void(const std::vector<ServiceListenerEntry> &));
    };

    class MockServiceListenerEntry : public cppmicroservices::ServiceListenerEntry
    {
      public:
        MockServiceListenerEntry() : ServiceListenerEntry() {}
        MockServiceListenerEntry(const ServiceListenerEntry & other) : ServiceListenerEntry(other) {}
        MockServiceListenerEntry(const ServiceListenerHook::ListenerInfo & info) : ServiceListenerEntry(info) {}
        MOCK_METHOD1(SetRemoved, void(bool));
        MockServiceListenerEntry(const std::shared_ptr<BundleContextPrivate> & context, const ServiceListener & l, void * data, ListenerTokenId tokenId, const std::string & filter) : ServiceListenerEntry(context, l, data, tokenId, filter) {}
        MOCK_METHOD0(GetLDAPExpr, const LDAPExpr &());
        MOCK_METHOD0(GetLocalCache, LDAPExpr::LocalCache &());
        MOCK_METHOD1(CallDelegate, void(const ServiceEvent &));
        MOCK_METHOD3(Contains, bool(const std::shared_ptr<BundleContextPrivate> &, const ServiceListener &, void *));
        MOCK_METHOD0(Id, ListenerTokenId());
        MOCK_METHOD0(Hash, std::size_t());
    };

    class MockServiceListeners : public cppmicroservices::ServiceListeners
    {
      public:
        MockServiceListeners(CoreBundleContext * coreCtx) : ServiceListeners(coreCtx) {}
        MOCK_METHOD0(Clear, void());
        MOCK_METHOD4(AddServiceListener, ListenerToken(const std::shared_ptr<BundleContextPrivate> &, const ServiceListener &, void *, const std::string &));
        MOCK_METHOD4(RemoveServiceListener, void(const std::shared_ptr<BundleContextPrivate> &, ListenerTokenId, const ServiceListener &, void *));
        MOCK_METHOD3(AddBundleListener, ListenerToken(const std::shared_ptr<BundleContextPrivate> &, const BundleListener &, void *));
        MOCK_METHOD3(RemoveBundleListener, void(const std::shared_ptr<BundleContextPrivate> &, const BundleListener &, void *));
        MOCK_METHOD3(AddFrameworkListener, ListenerToken(const std::shared_ptr<BundleContextPrivate> &, const FrameworkListener &, void *));
        MOCK_METHOD3(RemoveFrameworkListener, void(const std::shared_ptr<BundleContextPrivate> &, const FrameworkListener &, void *));
        MOCK_METHOD2(RemoveListener, void(const std::shared_ptr<BundleContextPrivate> &, ListenerToken));
        MOCK_METHOD1(SendFrameworkEvent, void(const FrameworkEvent &));
        MOCK_METHOD1(BundleChanged, void(const BundleEvent &));
        MOCK_METHOD1(RemoveAllListeners, void(const std::shared_ptr<BundleContextPrivate> &));
        MOCK_METHOD1(HooksBundleStopped, void(const std::shared_ptr<BundleContextPrivate> &));
        MOCK_METHOD3(ServiceChanged, void(ServiceListenerEntries &, const ServiceEvent &, ServiceListenerEntries &));
        MOCK_METHOD2(ServiceChanged, void(ServiceListenerEntries &, const ServiceEvent &));
        MOCK_METHOD2(GetMatchingServiceListeners, void(const ServiceEvent &, ServiceListenerEntries &));
        MOCK_METHOD0(GetListenerInfoCollection, std::vector<ServiceListenerHook::ListenerInfo>());
        MOCK_METHOD0(MakeListenerToken, ListenerToken());
        MOCK_METHOD1(RemoveFromCache_unlocked, void(const ServiceListenerEntry &));
        MOCK_METHOD1(CheckSimple_unlocked, void(const ServiceListenerEntry &));
        MOCK_METHOD4(AddToSet_unlocked, void(ServiceListenerEntries &, const ServiceListenerEntries &, int, const std::string &));
        MOCK_METHOD3(RemoveLegacyServiceListenerAndNotifyHooks, void(const std::shared_ptr<BundleContextPrivate> &, const ServiceListener &, void *));
    };

    class MockServiceObjectsBase : public cppmicroservices::ServiceObjectsBase
    {
      public:
        MOCK_METHOD0(GetService, std::shared_ptr<void>());
        MOCK_METHOD0(GetServiceInterfaceMap, InterfaceMapConstPtr());
        MOCK_METHOD0(GetReference, ServiceReferenceBase());
    };

    class MockServiceRegistrationBase : public cppmicroservices::ServiceRegistrationBase
    {
      public:
        MOCK_METHOD1(GetReference, ServiceReferenceBase(const std::string &));
        MOCK_METHOD1(SetProperties, void(const ServiceProperties &));
        MOCK_METHOD1(SetProperties, void(ServiceProperties &&));
        MOCK_METHOD0(Unregister, void());
        MOCK_METHOD0(LockServiceRegistration, ServiceRegistrationLocks());
    };

    class MockServiceRegistry : public cppmicroservices::ServiceRegistry
    {
      public:
        MOCK_METHOD0(Clear, void());
        MOCK_METHOD5(CreateServiceProperties, Properties(const ServiceProperties &, const std::vector<std::string> &, bool, bool, long));
        MockServiceRegistry(CoreBundleContext * coreCtx) : ServiceRegistry(coreCtx) {}
        MOCK_METHOD3(RegisterService, ServiceRegistrationBase(BundlePrivate *, const InterfaceMapConstPtr &, const ServiceProperties &));
        MOCK_METHOD1(UpdateServiceRegistrationOrder, void(const std::vector<std::string> &));
        MOCK_METHOD2(Get, void(const std::string &, std::vector<ServiceRegistrationBase> &));
        MOCK_METHOD2(Get, ServiceReferenceBase(BundlePrivate *, const std::string &));
        MOCK_METHOD4(Get, void(const std::string &, const std::string &, BundlePrivate *, std::vector<ServiceReferenceBase> &));
        MOCK_METHOD1(RemoveServiceRegistration, void(const ServiceRegistrationBase &));
        MOCK_METHOD2(GetRegisteredByBundle, void(BundlePrivate *, std::vector<ServiceRegistrationBase> &));
        MOCK_METHOD2(GetUsedByBundle, void(BundlePrivate *, std::vector<ServiceRegistrationBase> &));
        MOCK_METHOD1(RemoveServiceRegistration_unlocked, void(const ServiceRegistrationBase &));
        MOCK_METHOD2(Get_unlocked, void(const std::string &, std::vector<ServiceRegistrationBase> &));
        MOCK_METHOD4(Get_unlocked, void(const std::string &, const std::string &, BundlePrivate *, std::vector<ServiceReferenceBase> &));
    };

    template <class S, class T = S>
    class MockServiceTracker : public cppmicroservices::ServiceTracker<S, T>
    {
      public:
        using TrackedParamType = typename ServiceTrackerCustomizer<S, T>::TrackedParamType;
        using TrackingMap = std::unordered_map<ServiceReference<S>, std::shared_ptr<TrackedParamType>>;
        MockServiceTracker(const BundleContext & context, const ServiceReference<S> & reference, ServiceTrackerCustomizer<S, T> * customizer) : ServiceTracker<S, T>(context, reference, customizer) {}
        MockServiceTracker(const BundleContext & context, const std::string & clazz, ServiceTrackerCustomizer<S, T> * customizer) : ServiceTracker<S, T>(context, clazz, customizer) {}
        MockServiceTracker(const BundleContext & context, const LDAPFilter & filter, ServiceTrackerCustomizer<S, T> * customizer) : ServiceTracker<S, T>(context, filter, customizer) {}
        MockServiceTracker(const BundleContext & context, ServiceTrackerCustomizer<S, T> * customizer) : ServiceTracker<S, T>(context, customizer) {}
        MOCK_METHOD0(Open, void());
        MOCK_METHOD0(Close, void());
        MOCK_METHOD0(WaitForService, std::shared_ptr<TrackedParamType>());
        MOCK_METHOD0(GetServiceReferences, std::vector<ServiceReference<S>>());
        MOCK_METHOD0(GetServiceReference, ServiceReference<S>());
        MOCK_METHOD1(GetService, std::shared_ptr<TrackedParamType>(const ServiceReference<S> &));
        MOCK_METHOD0(GetServices, std::vector<std::shared_ptr<TrackedParamType>>());
        MOCK_METHOD0(GetService, std::shared_ptr<TrackedParamType>());
        MOCK_METHOD1(Remove, void(const ServiceReference<S> &));
        MOCK_METHOD0(Size, int());
        MOCK_METHOD0(GetTrackingCount, int());
        MOCK_METHOD1(GetTracked, void(TrackingMap &));
        MOCK_METHOD0(IsEmpty, bool());
        MOCK_METHOD1(AddingService, std::shared_ptr<TrackedParamType>(const ServiceReference<S> &));
        MOCK_METHOD2(ModifiedService, void(const ServiceReference<S> &, const std::shared_ptr<TrackedParamType> &));
        MOCK_METHOD2(RemovedService, void(const ServiceReference<S> &, const std::shared_ptr<TrackedParamType> &));
    };

    template <class S, class T = S>
    class MockServiceTrackerCustomizer : public cppmicroservices::ServiceTrackerCustomizer<S, T>
    {
      public:
        using TrackedType = T;
        using TrackedParamType = typename ServiceTrackerCustomizer<S, T>::TrackedParamType;
        MOCK_METHOD1(ConvertToTrackedType, std::shared_ptr<TrackedType>(const std::shared_ptr<S> &));
        MOCK_METHOD1(AddingService, std::shared_ptr<TrackedParamType>(const ServiceReference<S> &));
        MOCK_METHOD2(ModifiedService, void(const ServiceReference<S> &, const std::shared_ptr<TrackedParamType> &));
        MOCK_METHOD2(RemovedService, void(const ServiceReference<S> &, const std::shared_ptr<TrackedParamType> &));
    };

    class MockSharedLibrary : public cppmicroservices::SharedLibrary
    {
      public:
        MockSharedLibrary() : SharedLibrary() {}
        MockSharedLibrary(const SharedLibrary & other) : SharedLibrary(other) {}
        MockSharedLibrary(const std::string & libPath, const std::string & name) : SharedLibrary(libPath, name) {}
        MockSharedLibrary(const std::string & absoluteFilePath) : SharedLibrary(absoluteFilePath) {}
        MOCK_METHOD0(Load, void());
        MOCK_METHOD1(Load, void(int));
        MOCK_METHOD0(Unload, void());
        MOCK_METHOD1(SetName, void(const std::string &));
        MOCK_METHOD0(GetName, std::string());
        MOCK_METHOD1(GetFilePath, std::string(const std::string &));
        MOCK_METHOD1(SetFilePath, void(const std::string &));
        MOCK_METHOD0(GetFilePath, std::string());
        MOCK_METHOD1(SetLibraryPath, void(const std::string &));
        MOCK_METHOD0(GetLibraryPath, std::string());
        MOCK_METHOD1(SetSuffix, void(const std::string &));
        MOCK_METHOD0(GetSuffix, std::string());
        MOCK_METHOD1(SetPrefix, void(const std::string &));
        MOCK_METHOD0(GetPrefix, std::string());
        MOCK_METHOD0(GetHandle, void *());
        MOCK_METHOD0(IsLoaded, bool());
    };

    template <class T>
    class MockTrackedBundle : public cppmicroservices::detail::TrackedBundle<T>
    {
      public:
        MOCK_METHOD1(BundleChanged, void(const BundleEvent &));
        MOCK_METHOD0(WaitOnCustomizersToFinish, void());
        MOCK_METHOD0(Modified, void());
        MOCK_METHOD2(CustomizerAdding, std::optional<T>(Bundle, const BundleEvent &));
        MOCK_METHOD3(CustomizerModified, void(Bundle, const BundleEvent &, const T &));
        MOCK_METHOD3(CustomizerRemoved, void(Bundle, const BundleEvent &, const T &));
    };

    template <class S, class T>
    class MockTrackedService : public cppmicroservices::detail::TrackedService<S, T>
    {
      public:
        using TrackedParamType = typename ServiceTrackerCustomizer<S, T>::TrackedParamType;
        MOCK_METHOD1(ServiceChanged, void(const ServiceEvent &));
        MOCK_METHOD0(WaitOnCustomizersToFinish, void());
        MOCK_METHOD0(Modified, void());
        MOCK_METHOD2(CustomizerAdding, std::optional<std::shared_ptr<TrackedParamType>>(ServiceReference<S>, const ServiceEvent &));
        MOCK_METHOD3(CustomizerModified, void(ServiceReference<S>, const ServiceEvent &, const std::shared_ptr<TrackedParamType> &));
        MOCK_METHOD3(CustomizerRemoved, void(ServiceReference<S>, const ServiceEvent &, const std::shared_ptr<TrackedParamType> &));
    };

} // namespace cppmicroservices

#endif /* CPPMICROSERVICES_FRAMEWORK_MOCKS_H */
