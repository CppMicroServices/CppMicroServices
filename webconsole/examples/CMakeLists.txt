set(srcs usWebConsoleDriver.cpp)

add_definitions(-DUS_BUNDLE_NAME=main)

usFunctionGenerateBundleInit(srcs)
add_executable(us${PROJECT_NAME}Driver ${srcs})
target_link_libraries(us${PROJECT_NAME}Driver usHttpService)
