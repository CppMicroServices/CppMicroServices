# sources and headers
add_subdirectory(src)

set(_ds_private_headers)
foreach(_header ${_private_headers})
  list(APPEND _ds_private_headers ${CMAKE_CURRENT_SOURCE_DIR}/src/${_header})
endforeach()

# link libraries for the DeclarativeServices lib
set(_link_libraries )

if(UNIX)
  list(APPEND _link_libraries dl)
endif()

if(CMAKE_THREAD_LIBS_INIT)
  list(APPEND _link_libraries ${CMAKE_THREAD_LIBS_INIT})
endif()


# Configure the bundles manifest.json file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json.in
	       ${CMAKE_CURRENT_BINARY_DIR}/resources/manifest.json)

if(MINGW)
  # silence ignored attributes warnings
  add_compile_options(-Wno-attributes)
  
  # needed for boost::asio::thread_pool
  list(APPEND _link_libraries ws2_32.lib)
endif()

if (US_BUILD_TESTING)
  add_compile_options(-DUSING_GTEST)
endif()

add_compile_definitions(BOOST_DATE_TIME_NO_LIB)
add_compile_definitions(BOOST_REGEX_NO_LIB)


usMacroCreateBundle(DeclarativeServices
  VERSION "1.5.5"
  DEPENDS Framework
  TARGET DeclarativeServices
  SYMBOLIC_NAME declarative_services
  EMBED_RESOURCE_METHOD LINK
  LINK_LIBRARIES ${_link_libraries} usServiceComponent usAsyncWorkService
  PRIVATE_HEADERS ${_ds_private_headers}
  SOURCES $<TARGET_OBJECTS:DeclarativeServicesObjs> src/SCRActivator.cpp
  BINARY_RESOURCES manifest.json
  )

include_directories(${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
  ${CppMicroServices_BINARY_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_BINARY_DIR}/compendium/CM/include
  ${CppMicroServices_BINARY_DIR}/compendium/cppmicroservices/cm/include
  ${CppMicroServices_SOURCE_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/AsyncWorkService/include
  ${CppMicroServices_SOURCE_DIR}/compendium/CM/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googletest/include
  ${CppMicroServices_SOURCE_DIR}/third_party/googletest/googlemock/include
  ${CppMicroServices_SOURCE_DIR}/cppmicroservices/cm/include
  )

# There are warnings in the boost asio headers which are flagged as errors. Include the boost
# asio headers as system headers to ignore these warnings and not treat them as errors.
include_directories(SYSTEM ${CppMicroServices_SOURCE_DIR}/third_party/boost/include)
