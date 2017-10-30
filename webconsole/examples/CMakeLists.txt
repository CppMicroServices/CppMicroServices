find_package(CppMicroServices COMPONENTS HttpService REQUIRED)

set(srcs WebConsoleDriver.cpp)

add_executable(us${PROJECT_NAME}Driver ${srcs})
target_link_libraries(us${PROJECT_NAME}Driver ${US_LIBRARIES})
