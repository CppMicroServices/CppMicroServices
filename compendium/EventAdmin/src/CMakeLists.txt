set(_srcs
  EventAdminImpl.cpp
  EMLogger.cpp
)

set(_private_headers
  EMActivator.hpp
  EventAdminImpl.hpp
  EventAdminPrivate.hpp
  EMLogger.hpp
)

add_library(EventAdminObjs OBJECT ${_srcs} ${_private_headers})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  get_property(_compile_flags TARGET EventAdminObjs PROPERTY COMPILE_FLAGS)
  set_property(TARGET EventAdminObjs PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")
endif()

target_include_directories(EventAdminObjs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_BINARY_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/EM/include
  ${CppMicroServices_BINARY_DIR}/compendium/EM/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  )

# There are warnings in the boost asio headers which are flagged as errors. Include the boost
# asio headers as system headers to ignore these warnings and not treat them as errors.
include_directories(SYSTEM ${CppMicroServices_SOURCE_DIR}/third_party/boost/include)
