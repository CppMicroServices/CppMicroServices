#-----------------------------------------------------------------------------
# Link to GTest Libraries
#-----------------------------------------------------------------------------

if(GTEST_INCLUDE_DIRS)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(UnitTests main.cpp)
  target_link_libraries(UnitTests ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(UnitTests ${Framework_TARGET})
endif()
