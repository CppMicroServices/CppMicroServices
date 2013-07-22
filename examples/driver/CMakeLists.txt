
if(WIN32)
  string(REPLACE "/" "\\\\" CMAKE_LIBRARY_OUTPUT_DIRECTORY_NATIVE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  string(REPLACE "/" "\\\\" CMAKE_RUNTIME_OUTPUT_DIRECTORY_NATIVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_NATIVE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_NATIVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/usCppMicroServicesExampleDriverConfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/usCppMicroServicesExampleDriverConfig.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(CppMicroServicesExampleDriver main.cpp)
target_link_libraries(CppMicroServicesExampleDriver ${CppMicroServices_LIBRARIES})
