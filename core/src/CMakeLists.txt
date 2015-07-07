#-----------------------------------------------------------------------------
# Source files
#-----------------------------------------------------------------------------

set(_srcs
  util/usAny.cpp
  util/usLDAPProp.cpp
  util/usSharedLibrary.cpp
  util/usUtils.cpp

  service/usLDAPExpr.cpp
  service/usLDAPFilter.cpp
  service/usServiceException.cpp
  service/usServiceEvent.cpp
  service/usServiceEventListenerHook.cpp
  service/usServiceFindHook.cpp
  service/usServiceHooks.cpp
  service/usServiceListenerEntry.cpp
  service/usServiceListenerEntry_p.h
  service/usServiceListenerHook.cpp
  service/usServiceListeners.cpp
  service/usServiceListeners_p.h
  service/usServiceObjects.cpp
  service/usServiceProperties.cpp
  service/usServicePropertiesImpl.cpp
  service/usServiceReferenceBase.cpp
  service/usServiceReferenceBasePrivate.cpp
  service/usServiceRegistrationBase.cpp
  service/usServiceRegistrationBasePrivate.cpp
  service/usServiceRegistry.cpp
  service/usServiceRegistry_p.h

  module/usCoreModuleActivator.cpp
  module/usCoreModuleContext_p.h
  module/usCoreModuleContext.cpp
  module/usModuleContext.cpp
  module/usModule.cpp
  module/usModuleEvent.cpp
  module/usModuleEventHook.cpp
  module/usModuleFindHook.cpp
  module/usModuleHooks.cpp
  module/usModuleInfo.cpp
  module/usModuleManifest.cpp
  module/usModulePrivate.cpp
  module/usModuleRegistry.cpp
  module/usModuleResource.cpp
  module/usModuleResourceBuffer.cpp
  module/usModuleResourceContainer.cpp
  module/usModuleResourceStream.cpp
  module/usModuleSettings.cpp
  module/usModuleUtils.cpp
  module/usModuleVersion.cpp

  ../../third_party/jsoncpp.cpp
  ../../third_party/miniz.c
)

set(_private_headers
  util/usAtomicInt_p.h
  util/usListenerFunctors_p.h
  util/usLog_p.h
  util/usStaticInit_p.h
  util/usThreads_p.h
  util/usUtils_p.h
  util/usWaitCondition_p.h

  service/usServiceHooks_p.h
  service/usServiceListenerHook_p.h
  service/usServicePropertiesImpl_p.h
  service/usServiceTracker.tpp
  service/usServiceTrackerPrivate.h
  service/usServiceTrackerPrivate.tpp
  service/usTrackedService_p.h
  service/usTrackedServiceListener_p.h
  service/usTrackedService.tpp

  module/usModuleAbstractTracked_p.h
  module/usModuleAbstractTracked.tpp
  module/usModuleHooks_p.h
  module/usModuleResourceBuffer_p.h
  module/usModuleResourceContainer_p.h
  module/usModuleUtils_p.h
)

