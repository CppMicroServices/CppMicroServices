
set(_srcs
  ComponentContextImpl.cpp
  ComponentRegistry.cpp
  SCRBundleExtension.cpp
  SCRLogger.cpp
  ServiceComponentRuntimeImpl.cpp
  ComponentContextImpl.cpp
  manager/BundleLoader.cpp
  manager/BundleOrPrototypeComponentConfiguration.cpp
  manager/ComponentConfigurationFactory.cpp
  manager/ComponentConfigurationImpl.cpp
  manager/ComponentManagerImpl.cpp
  manager/ReferenceManagerImpl.cpp
  manager/RegistrationManager.cpp
  manager/SingletonComponentConfiguration.cpp
  manager/states/CCActiveState.cpp
  manager/states/CCRegisteredState.cpp
  manager/states/CCSatisfiedState.cpp
  manager/states/CCUnsatisfiedReferenceState.cpp
  manager/states/CMDisabledState.cpp
  manager/states/CMEnabledState.cpp
  metadata/MetadataParserImpl.cpp
  metadata/ReferenceMetadata.cpp
  metadata/ServiceMetadata.cpp
  metadata/Util.cpp
  )

set(_private_headers
  ComponentContextImpl.hpp
  ComponentRegistry.hpp
  SCRActivator.hpp
  SCRBundleExtension.hpp
  SCRLogger.hpp
  ServiceComponentRuntimeImpl.hpp
  ServiceReferenceComparator.hpp
  manager/BundleLoader.hpp
  manager/BundleOrPrototypeComponentConfiguration.hpp
  manager/ComponentConfiguration.hpp
  manager/ComponentConfigurationFactory.hpp
  manager/ComponentConfigurationImpl.hpp
  manager/ComponentManager.hpp
  manager/ComponentManagerImpl.hpp
  manager/ConcurrencyUtil.hpp
  manager/ReferenceManager.hpp
  manager/ReferenceManagerImpl.hpp
  manager/RegistrationManager.hpp
  manager/SingletonComponentConfiguration.hpp
  manager/states/CCActiveState.hpp
  manager/states/CCRegisteredState.hpp
  manager/states/CCSatisfiedState.hpp
  manager/states/CCUnsatisfiedReferenceState.hpp
  manager/states/CMDisabledState.hpp
  manager/states/CMEnabledState.hpp
  manager/states/ComponentConfigurationState.hpp
  manager/states/ComponentManagerState.hpp
  metadata/ComponentMetadata.hpp
  metadata/MetadataParser.hpp
  metadata/MetadataParserFactory.hpp
  metadata/MetadataParserImpl.hpp
  metadata/ReferenceMetadata.hpp
  metadata/ServiceMetadata.hpp
  metadata/Util.hpp
  )
  
add_library(DeclarativeServicesObjs OBJECT ${_srcs} ${_private_headers})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  get_property(_compile_flags TARGET DeclarativeServicesObjs PROPERTY COMPILE_FLAGS)
  set_property(TARGET DeclarativeServicesObjs PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")
endif()

include_directories(${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )

