set(srcs usWebConsoleDriver.cpp)

add_definitions(-DUS_MODULE_NAME=main)

usFunctionGenerateModuleInit(srcs)
add_executable(us${PROJECT_NAME}Driver ${srcs})
target_link_libraries(us${PROJECT_NAME}Driver usHttpService)
