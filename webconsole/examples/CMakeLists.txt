find_package(CppMicroServices COMPONENTS HttpService REQUIRED)

include_directories(${US_INCLUDE_DIRS})

set(srcs WebConsoleDriver.cpp)

add_executable(us${PROJECT_NAME}Driver ${srcs})
target_link_libraries(us${PROJECT_NAME}Driver ${US_LIBRARIES})
