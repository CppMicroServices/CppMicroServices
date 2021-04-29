set(_srcs
  CMBundleExtension.cpp
  CMLogger.cpp
  ConfigurationAdminImpl.cpp
  ConfigurationImpl.cpp
  )

set(_private_headers
  CMActivator.hpp
  CMBundleExtension.hpp
  CMLogger.hpp
  ConfigurationAdminImpl.hpp
  ConfigurationAdminPrivate.hpp
  ConfigurationImpl.hpp
  ConfigurationPrivate.hpp
  )

add_library(ConfigurationAdminObjs OBJECT ${_srcs} ${_private_headers})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  get_property(_compile_flags TARGET ConfigurationAdminObjs PROPERTY COMPILE_FLAGS)
  set_property(TARGET ConfigurationAdminObjs PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")
endif()

target_include_directories(ConfigurationAdminObjs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  ${CppMicroServices_SOURCE_DIR}/util/include
  ${CppMicroServices_BINARY_DIR}/util/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )
