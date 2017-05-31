#-----------------------------------------------------------------------------
# Link to GTest Libraries
#-----------------------------------------------------------------------------

if(GTEST_INCLUDE_DIRS)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(UnitTests main.cpp)
  target_link_libraries(UnitTests ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(UnitTests ${Framework_TARGET})

  # This copies the Google Test libraries into the same folder as the
  # executable so that they can be seen at runtime on Windows.
  # Mac and Linux use RPATHs and do not need to do this.
  if (MSVC)
    foreach( input_file ${GTEST_BOTH_LIBRARIES})
      string(REPLACE ".lib" ".dll" input_dll_file ${input_file})
      add_custom_command(TARGET UnitTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${input_dll_file}"
            $<TARGET_FILE_DIR:UnitTests>)
    endforeach( input_file)
  endif(MSVC)
endif()

