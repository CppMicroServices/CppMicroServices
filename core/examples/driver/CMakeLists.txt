
if(WIN32)
  string(REPLACE "/" "\\\\" CMAKE_LIBRARY_OUTPUT_DIRECTORY_NATIVE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  string(REPLACE "/" "\\\\" CMAKE_RUNTIME_OUTPUT_DIRECTORY_NATIVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_NATIVE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_NATIVE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/us${PROJECT_NAME}DriverConfig.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/us${PROJECT_NAME}DriverConfig.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(us${PROJECT_NAME}Driver main.cpp)
if(NOT US_BUILD_SHARED_LIBS)
  target_link_libraries(us${PROJECT_NAME}Driver Example-eventlistener Example-dictionaryservice Example-dictionaryclient)
endif()
target_link_libraries(us${PROJECT_NAME}Driver ${CppMicroServices_LIBRARIES})
