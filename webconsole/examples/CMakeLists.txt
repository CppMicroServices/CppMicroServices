set(srcs usWebConsoleDriver.cpp)

usFunctionGenerateExecutableInit(srcs IDENTIFIER us${PROJECT_NAME}Driver)
add_executable(us${PROJECT_NAME}Driver ${srcs})
target_link_libraries(us${PROJECT_NAME}Driver usHttpService)
