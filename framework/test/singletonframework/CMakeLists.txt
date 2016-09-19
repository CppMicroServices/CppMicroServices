# Create a non-bundle shared library for the sole
# purpose of testing static destruction on Windows.
#
project(singletonframework)

find_package(CppMicroServices REQUIRED Framework QUIET
    HINTS ${CppMicroServices_BINARY_DIR}
    NO_DEFAULT_PATH
  )

set(_srcs singletonframework.cpp)
set(_includes singletonframework.h)

include_directories(${US_INCLUDE_DIRS})

add_library(${PROJECT_NAME} SHARED ${_srcs} ${_includes})
target_link_libraries(${PROJECT_NAME} ${US_LIBRARIES})
