
set(_srcs
  ConfigurationListenerImpl.cpp
  ComponentContextImpl.cpp
  ComponentRegistry.cpp
  SCRAsyncWorkService.cpp
  SCRBundleExtension.cpp
  SCRExtensionRegistry.cpp
  SCRLogger.cpp
  ServiceComponentRuntimeImpl.cpp
  ComponentContextImpl.cpp
  manager/BundleLoader.cpp
  manager/BundleOrPrototypeComponentConfiguration.cpp
  manager/ComponentConfigurationFactory.cpp
  manager/ComponentConfigurationImpl.cpp
  manager/ComponentFactoryImpl.cpp
  manager/ComponentManagerImpl.cpp
  manager/ConfigurationManager.cpp
  manager/ConfigurationNotifier.cpp
  manager/ReferenceManagerImpl.cpp
  manager/RegistrationManager.cpp
  manager/SingletonComponentConfiguration.cpp
  manager/BindingPolicy.cpp
  manager/BindingPolicyStaticReluctant.cpp
  manager/BindingPolicyStaticGreedy.cpp
  manager/BindingPolicyDynamicReluctant.cpp
  manager/BindingPolicyDynamicGreedy.cpp
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
  ConfigurationListenerImpl.hpp
  ComponentContextImpl.hpp
  ComponentRegistry.hpp
  SCRActivator.hpp
  SCRAsyncWorkService.hpp
  SCRBundleExtension.hpp
  SCRExtensionRegistry.hpp
  SCRLogger.hpp
  ServiceComponentRuntimeImpl.hpp
  ServiceReferenceComparator.hpp
  manager/BundleLoader.hpp
  manager/BundleOrPrototypeComponentConfiguration.hpp
  manager/ComponentConfiguration.hpp
  manager/ComponentConfigurationFactory.hpp
  manager/ComponentConfigurationImpl.hpp
  manager/ComponentFactoryImpl.hpp
  manager/ComponentManager.hpp
  manager/ComponentManagerImpl.hpp
  manager/ConfigurationManager.hpp
  manager/ConfigurationNotifier.hpp
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

add_compile_definitions(US_BUNDLE_NAME=${${PROJECT_NAME}_SYMBOLIC_NAME})
add_compile_definitions(BOOST_DATE_TIME_NO_LIB)
add_compile_definitions(BOOST_REGEX_NO_LIB)

include_directories(${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  ${CppMicroServices_SOURCE_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )

usFunctionBoostPath(BOOST_SYSTEM ${US_USE_SYSTEM_BOOST} CPPMS_SOURCE_DIR ${CppMicroServices_SOURCE_DIR} BOOST_DIR ${BOOST_INCLUDEDIR})

# There are warnings in the boost asio headers which are flagged as errors. Include the boost
# asio headers as system headers to ignore these warnings and not treat them as errors.
include_directories(SYSTEM ${_boost_library})
