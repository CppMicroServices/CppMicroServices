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
  if (WIN32)
    foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
      get_filename_component(dir ${lib_fullpath} DIRECTORY)
      get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
      set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      add_custom_command(TARGET UnitTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${dll_file}"
            $<TARGET_FILE_DIR:UnitTests>)
    endforeach(lib_fullpath)
  endif(WIN32)
endif()

