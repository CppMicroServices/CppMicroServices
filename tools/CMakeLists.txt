
include_directories(${US_INCLUDE_DIRS})

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_NAME}\")

add_executable(${US_RCC_EXECUTABLE_NAME} usResourceCompiler.cpp)
