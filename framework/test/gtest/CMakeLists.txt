#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(us_gtest_test_exe_name FrameworkTests)

include_directories(${GTEST_INCLUDE_DIRS})

set(_gtest_tests 
  BundleVersionTest.cpp
)

add_executable(${us_gtest_test_exe_name} ${_gtest_tests})

target_link_libraries(${us_gtest_test_exe_name} ${GTEST_BOTH_LIBRARIES})
target_link_libraries(${us_gtest_test_exe_name} ${Framework_TARGET})

# Run the GTest EXE from ctest.
add_test(NAME ${us_gtest_test_exe_name} 
  COMMAND ${us_gtest_test_exe_name}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32 AND US_USE_SYSTEM_GTEST)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${us_gtest_test_exe_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:${us_gtest_test_exe_name}>)
  endforeach(lib_fullpath)
endif()

