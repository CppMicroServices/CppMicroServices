project(singletonframework)

find_package(CppMicroServices COMPONENTS Framework REQUIRED)

set(_srcs singletonframework.cpp)
set(_includes singletonframework.h)

include_directories(${US_INCLUDE_DIRS} ${_includes})

add_library(${PROJECT_NAME} SHARED ${_srcs} ${_includes})
target_link_libraries(${PROJECT_NAME} ${US_LIBRARIES})
