#-----------------------------------------------------------------------------
# Link to GTest Libraries
#-----------------------------------------------------------------------------

if(GTEST_INCLUDE_DIRS)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(UnitTests main.cpp)
  target_link_libraries(UnitTests ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(UnitTests ${Framework_TARGET})

  # Copy the dlls to the binary directory of UnitTests
  # in Windows. This is so that the dlls can be picked up
  # at load time.
  if (MSVC)
    set( GTEST_DLLS
    gtest.dll
    gtest.lib
    gtest_main.dll
    gtest_main.lib
    )

    foreach( input_file ${GTEST_DLLS})
    add_custom_command(TARGET UnitTests POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${GTEST_ROOT}/lib/${input_file}"
          $<TARGET_FILE_DIR:UnitTests>)
    endforeach( input_file)
  endif(MSVC)
endif()