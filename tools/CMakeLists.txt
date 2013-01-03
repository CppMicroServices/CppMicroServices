
if(NOT US_IS_EMBEDDED)
  include_directories(${US_INTERNAL_INCLUDE_DIRS})
endif()

add_definitions(-DUS_RCC_EXECUTABLE_NAME=\"${US_RCC_EXECUTABLE_NAME}\")

add_executable(${US_RCC_EXECUTABLE_NAME} usResourceCompiler.cpp)
