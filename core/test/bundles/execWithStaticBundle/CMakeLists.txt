include_directories(${CMAKE_CURRENT_BINARY_DIR})

# This test is only applicable when build mode is shared libraries. 
# The test point usExecutableBundleTest in usCoreDriver is sufficeint for static library builds.
if(US_BUILD_SHARED_LIBS)

set(BUILD_SHARED_LIBS 0)
usFunctionCreateTestBundleWithResources(TestStaticBundle
  SOURCES usTestStaticBundle.cpp
  RESOURCES manifest.json
  RESOURCES_ROOT resources_static
  SKIP_BUNDLE_LIST)

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})

add_executable(usTestExecutableWithStaticBundle main.cpp)

target_link_libraries(usTestExecutableWithStaticBundle ${Core_TARGET} TestStaticBundle)


add_custom_command(
      TARGET usTestExecutableWithStaticBundle
      POST_BUILD
      COMMAND ${US_RCC_EXECUTABLE_NAME} -b $<TARGET_FILE:usTestExecutableWithStaticBundle> -z $<TARGET_FILE:TestStaticBundle>
      WORKING_DIRECTORY ${US_RESOURCE_WORKING_DIRECTORY}
      COMMENT "Appending zipped resources to usTestExecutableWithStaticBundle"
      VERBATIM
    )

if(US_BUILD_TESTING)
  enable_testing()
  add_test(NAME usTestExecutableWithStaticBundle
      WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
      COMMAND usTestExecutableWithStaticBundle)
endif()

endif()