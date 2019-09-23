#-----------------------------------------------------------------------------
# Source files
#-----------------------------------------------------------------------------

set(_srcs
  util/Any.cpp
  util/AnyMap.cpp
  util/Framework.cpp
  util/FrameworkEvent.cpp
  util/FrameworkFactory.cpp
  util/FrameworkPrivate.cpp
  util/LDAPExpr.cpp
  util/LDAPFilter.cpp
  util/LDAPProp.cpp
  util/Properties.cpp
  util/SharedLibrary.cpp
  util/Utils.cpp

  service/ListenerToken.cpp
  service/ServiceException.cpp
  service/ServiceEvent.cpp
  service/ServiceEventListenerHook.cpp
  service/ServiceFindHook.cpp
  service/ServiceHooks.cpp
  service/ServiceListenerEntry.cpp
  service/ServiceListenerHook.cpp
  service/ServiceListeners.cpp
  service/ServiceObjects.cpp
  service/ServiceReferenceBase.cpp
  service/ServiceReferenceBasePrivate.cpp
  service/ServiceRegistrationBase.cpp
  service/ServiceRegistrationBasePrivate.cpp
  service/ServiceRegistry.cpp

  bundle/Bundle.cpp
  bundle/BundleArchive.cpp
  bundle/BundleContext.cpp
  bundle/BundleContextPrivate.cpp
  bundle/BundleEvent.cpp
  bundle/BundleEventHook.cpp
  bundle/BundleFindHook.cpp
  bundle/BundleHooks.cpp
  bundle/BundleManifest.cpp
  bundle/BundlePrivate.cpp
  bundle/BundleRegistry.cpp
  bundle/BundleResource.cpp
  bundle/BundleResourceBuffer.cpp
  bundle/BundleResourceContainer.cpp
  bundle/BundleResourceStream.cpp
  bundle/BundleStorageFile.cpp
  bundle/BundleStorageMemory.cpp
  bundle/BundleThread.cpp
  bundle/BundleUtils.cpp
  bundle/BundleVersion.cpp
  bundle/Constants.cpp
  bundle/CoreBundleContext.cpp
  bundle/Debug.cpp
  ../../third_party/jsoncpp.cpp
  ../../third_party/miniz.c

  ../../third_party/absl/base/log_severity.cc
  ../../third_party/absl/base/internal/raw_logging.cc
  ../../third_party/absl/base/internal/throw_delegate.cc

  ../../third_party/absl/numeric/int128.cc

  ../../third_party/absl/strings/ascii.cc
  ../../third_party/absl/strings/charconv.cc
  ../../third_party/absl/strings/match.cc
  ../../third_party/absl/strings/numbers.cc
  ../../third_party/absl/strings/str_cat.cc
  ../../third_party/absl/strings/string_view.cc
  ../../third_party/absl/strings/internal/charconv_bigint.cc
  ../../third_party/absl/strings/internal/charconv_parse.cc
  ../../third_party/absl/strings/internal/memutil.cc
)

set(_private_headers
  util/FrameworkPrivate.h
  util/LDAPExpr.h
  util/Properties.h
  util/Utils.h

  service/ServiceHooks.h
  service/ServiceListenerEntry.h
  service/ServiceListenerHookPrivate.h
  service/ServiceListeners.h
  service/ServiceReferenceBasePrivate.h
  service/ServiceRegistrationBasePrivate.h
  service/ServiceRegistry.h

  bundle/BundleArchive.h
  bundle/BundleContextPrivate.h
  bundle/BundleEventInternal.h
  bundle/BundleHooks.h
  bundle/BundleManifest.h
  bundle/BundlePrivate.h
  bundle/BundleRegistry.h
  bundle/BundleResourceContainer.h
  bundle/BundleStorage.h
  bundle/BundleStorageFile.h
  bundle/BundleStorageMemory.h
  bundle/BundleThread.h
  bundle/BundleUtils.h
  bundle/CoreBundleContext.h
  bundle/Debug.h
  bundle/Fragment.h
  bundle/Resolver.h

  ../../third_party/absl/base/attributes.h
  ../../third_party/absl/base/config.h
  ../../third_party/absl/base/log_severity.h
  ../../third_party/absl/base/macros.h
  ../../third_party/absl/base/optimization.h
  ../../third_party/absl/base/policy_checks.h
  ../../third_party/absl/base/port.h
  ../../third_party/absl/base/internal/atomic_hook.h
  ../../third_party/absl/base/internal/raw_logging.h
  ../../third_party/absl/base/internal/throw_delegate.h

  ../../third_party/absl/strings/ascii.h
  ../../third_party/absl/strings/numbers.h
  ../../third_party/absl/strings/string_view.h
  ../../third_party/absl/strings/internal/memutil.h
)

# Turn off warnings when compiling Abseil files
file(GLOB_RECURSE ABSL_FILES 
  "${CMAKE_SOURCE_DIR}/third_party/absl/*.cc" 
  "${CMAKE_SOURCE_DIR}/third_party/absl/*.c" 
  "${CMAKE_SOURCE_DIR}/third_party/absl/*.h")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR 
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" 
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CF "-w")
  set_source_files_properties(
    ${ABSL_FILES}
    PROPERTIES
    COMPILE_FLAGS ${CF}
  ) 
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR 
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" OR 
      "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

