
#configure_file(CustomInstallScript.cmake.in ${PROJECT_BINARY_DIR}/CustomInstallScript.cmake)

#set(example_targets )
#  add_custom_command(TARGET ${_example_target} PRE_BUILD
#           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#           COMMAND ${CMAKE_COMMAND} -P ${PROJECT_BINARY_DIR}/CustomInstallScript.cmake
#           WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
#           COMMENT "Running CustomInstallScript.cmake"
#           VERBATIM)

project(CppMicroServicesExamples)

cmake_minimum_required(VERSION 2.8)

find_package(CppMicroServices NO_MODULE REQUIRED)

include_directories(${CppMicroServices_INCLUDE_DIRS})

function(CreateExample _name)
  add_library(${example_target_prefix}${_name} SHARED ${ARGN})
  if(${_name}_DEPENDS)
    foreach(_dep ${${_name}_DEPENDS})
      include_directories(${CppMicroServicesExamples_SOURCE_DIR}/${_dep})
      target_link_libraries(${example_target_prefix}${_name} ${example_target_prefix}${_dep})
    endforeach()
  endif()
  target_link_libraries(${example_target_prefix}${_name} ${CppMicroServices_LIBRARIES})
  set_target_properties(${example_target_prefix}${_name} PROPERTIES
    LABELS Examples
    OUTPUT_NAME ${_name}
  )
endfunction()

add_subdirectory(eventlistener)
add_subdirectory(dictionaryservice)
add_subdirectory(frenchdictionary)
add_subdirectory(dictionaryclient)
add_subdirectory(dictionaryclient2)
add_subdirectory(dictionaryclient3)
add_subdirectory(spellcheckservice)
add_subdirectory(spellcheckclient)
add_subdirectory(driver)

#-----------------------------------------------------------------------------
# Test if examples compile against an install tree and if the
# Makefile example compiles
#-----------------------------------------------------------------------------

if(US_BUILD_TESTING)
  enable_testing()

  set(_example_tests )

  set(_install_dir "${CppMicroServices_BINARY_DIR}/install_test/${CMAKE_INSTALL_PREFIX}")

  add_test(NAME usInstallCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${_install_dir}")

  add_test(NAME usInstallTest
           WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
           COMMAND ${CMAKE_COMMAND} --build ${CppMicroServices_BINARY_DIR} --target install)
  set_tests_properties(usInstallTest PROPERTIES
                       ENVIRONMENT "DESTDIR=${CppMicroServices_BINARY_DIR}/install_test"
                       DEPENDS usInstallCleanTest)

  set(_examples_binary_dir "${CppMicroServices_BINARY_DIR}/examples_build")

  add_test(NAME usExamplesCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${_examples_binary_dir}")

  add_test(NAME usExamplesCreateDirTest
           COMMAND ${CMAKE_COMMAND} -E make_directory "${_examples_binary_dir}")
  set_tests_properties(usExamplesCreateDirTest PROPERTIES
                       DEPENDS usExamplesCleanTest)

  add_test(NAME usExamplesConfigureTest
           WORKING_DIRECTORY ${_examples_binary_dir}
           COMMAND ${CMAKE_COMMAND}
                   -G ${CMAKE_GENERATOR}
                   -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                   "-DCppMicroServices_DIR:PATH=${_install_dir}/${CONFIG_CMAKE_DIR}"
                   "${CMAKE_CURRENT_LIST_DIR}")
  set_tests_properties(usExamplesConfigureTest PROPERTIES
                       DEPENDS "usInstallTest;usExamplesCreateDirTest")

  add_test(NAME usExamplesBuildTest
           WORKING_DIRECTORY ${_examples_binary_dir}
           COMMAND ${CMAKE_COMMAND} --build .)
  set_tests_properties(usExamplesBuildTest PROPERTIES
                       DEPENDS usExamplesConfigureTest)

  list(APPEND _example_tests usInstallCleanTest usInstallTest usExamplesCleanTest
       usExamplesCreateDirTest usExamplesConfigureTest usExamplesBuildTest)

  find_program(MAKE_COMMAND NAMES make gmake)
  find_program(CXX_COMMAND NAMES g++)
  mark_as_advanced(MAKE_COMMAND CXX_COMMAND)
  if(MAKE_COMMAND AND CXX_COMMAND)
    add_test(NAME usMakefileExampleCleanTest
             WORKING_DIRECTORY ${CppMicroServices_SOURCE_DIR}/examples/makefile
             COMMAND ${MAKE_COMMAND} clean)
    add_test(NAME usMakefileExampleTest
             WORKING_DIRECTORY ${CppMicroServices_SOURCE_DIR}/examples/makefile
             COMMAND ${MAKE_COMMAND})
    set_tests_properties(usMakefileExampleTest PROPERTIES
                         DEPENDS "usMakefileExampleCleanTest;usInstallTest"
                         ENVIRONMENT "CppMicroServices_ROOT=${CppMicroServices_BINARY_DIR}/install_test${CMAKE_INSTALL_PREFIX}")
    list(APPEND _example_tests usMakefileExampleCleanTest usMakefileExampleTest)
  endif()

  if(US_TEST_LABELS)
    set_tests_properties(${_example_tests} PROPERTIES LABELS "${US_TEST_LABELS}")
  endif()

endif()
