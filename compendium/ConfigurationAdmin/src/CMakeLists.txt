set(_srcs
  CMAsyncWorkService.cpp
  CMBundleExtension.cpp
  CMConstants.cpp
  CMLogger.cpp
  ConfigurationAdminImpl.cpp
  ConfigurationImpl.cpp
  ThreadpoolSafeFuturePrivate.cpp
  metadata/MetadataParserImpl.cpp
  )

set(_private_headers
  CMActivator.hpp
  CMAsyncWorkService.hpp
  CMBundleExtension.hpp
  CMConstants.hpp
  CMLogger.hpp
  ConfigurationAdminImpl.hpp
  ConfigurationAdminPrivate.hpp
  ConfigurationImpl.hpp
  ConfigurationPrivate.hpp
  ServiceReferenceComparator.hpp
  ThreadpoolSafeFuturePrivate.hpp
  SingleInvokeTask.hpp
  metadata/ConfigurationMetadata.hpp
  metadata/MetadataParser.hpp
  metadata/MetadataParserFactory.hpp
  metadata/MetadataParserImpl.hpp
  )

add_library(ConfigurationAdminObjs OBJECT ${_srcs} ${_private_headers})


get_property(_compile_flags TARGET ConfigurationAdminObjs PROPERTY COMPILE_FLAGS)
set_property(TARGET ConfigurationAdminObjs PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")

target_include_directories(ConfigurationAdminObjs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_BINARY_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )

usFunctionBoostPath(BOOST_SYSTEM ${US_USE_SYSTEM_BOOST} CPPMS_SOURCE_DIR ${CppMicroServices_SOURCE_DIR} BOOST_DIR ${BOOST_INCLUDEDIR})

# There are warnings in the boost asio headers which are flagged as errors. Include the boost
# asio headers as system headers to ignore these warnings and not treat them as errors.
include_directories(SYSTEM ${_boost_library})
