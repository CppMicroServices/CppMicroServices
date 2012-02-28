
include_directories(${US_INCLUDE_DIRS})
set(_link_libraries)
if(US_IS_EMBEDDED)
  set(_link_libraries ${US_EMBEDDING_LIBRARY})
else()
  set(_link_libraries ${PROJECT_NAME})
endif()

usFunctionCompileSnippets("${CMAKE_CURRENT_SOURCE_DIR}" ${_link_libraries})

