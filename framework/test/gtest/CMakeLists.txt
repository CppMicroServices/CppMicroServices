#-----------------------------------------------------------------------------
# Build and run the GTest Suite of tests
#-----------------------------------------------------------------------------

set(gtest_test_name FrameworkTests)

include_directories(${GTEST_INCLUDE_DIRS})

set(_gtest_tests 
  BundleVersionTest.cpp
)

add_executable(${gtest_test_name} ${_gtest_tests})

target_link_libraries(${gtest_test_name} ${GTEST_BOTH_LIBRARIES})
target_link_libraries(${gtest_test_name} ${Framework_TARGET})

add_test(NAME ${gtest_test_name} 
  COMMAND ${gtest_test_name}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy the Google Test libraries into the same folder as the
# executable so that they can be seen at runtime on Windows.
# Mac and Linux use RPATHs and do not need to do this.
if (WIN32)
  foreach(lib_fullpath ${GTEST_BOTH_LIBRARIES})
    get_filename_component(dir ${lib_fullpath} DIRECTORY)
    get_filename_component(name_no_ext ${lib_fullpath} NAME_WE)
    set(dll_file "${dir}/${name_no_ext}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    add_custom_command(TARGET ${gtest_test_name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${dll_file}"
        $<TARGET_FILE_DIR:${gtest_test_name}>)
  endforeach(lib_fullpath)
endif(WIN32)

