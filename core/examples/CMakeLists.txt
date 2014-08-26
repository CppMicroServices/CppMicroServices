project(CoreExamples)

cmake_minimum_required(VERSION 2.8)

find_package(CppMicroServices NO_MODULE REQUIRED)

include_directories(${CppMicroServices_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
# Set C/CXX flags
#-----------------------------------------------------------------------------

#if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CppMicroServices_CXX_FLAGS}")
#  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CppMicroServices_CXX_FLAGS_RELEASE}")
#  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CppMicroServices_CXX_FLAGS_DEBUG}")
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CppMicroServices_C_FLAGS}")
#  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CppMicroServices_C_FLAGS_RELEASE}")
#  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CppMicroServices_C_FLAGS_DEBUG}")
#endif()

#-----------------------------------------------------------------------------
# Init output directories
#-----------------------------------------------------------------------------

set(CoreExamples_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CoreExamples_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CoreExamples_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

foreach(_type ARCHIVE LIBRARY RUNTIME)
  if(NOT CMAKE_${_type}_OUTPUT_DIRECTORY)
    set(CMAKE_${_type}_OUTPUT_DIRECTORY ${CoreExamples_${_type}_OUTPUT_DIRECTORY})
  endif()
endforeach()


function(CreateExample _name)
  add_library(Example-${_name} ${ARGN})
  set_property(TARGET Example-${_name} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=${_name})
  if(NOT US_BUILD_SHARED_LIBS)
    set_property(TARGET Example-${_name} APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE)
  endif()

  if(${_name}_DEPENDS)
    foreach(_dep ${${_name}_DEPENDS})
      include_directories(${PROJECT_SOURCE_DIR}/${_dep})
      target_link_libraries(Example-${_name} Example-${_dep})
    endforeach()
  endif()
  target_link_libraries(Example-${_name} ${CppMicroServices_LIBRARIES})
  set_target_properties(Example-${_name} PROPERTIES
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

  if(WIN32)
    string(REGEX REPLACE "^.:" "" _install_prefix ${CMAKE_INSTALL_PREFIX})
  else()
    set(_install_prefix ${CMAKE_INSTALL_PREFIX})
  endif()
  set(_install_dir "${CppMicroServices_BINARY_DIR}/install_test/${_install_prefix}")

  add_test(NAME usInstallCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${_install_dir}")

  add_test(NAME usInstallTest
           WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
           COMMAND ${CMAKE_COMMAND} --build ${CppMicroServices_BINARY_DIR} --config $<CONFIGURATION> --target install)
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

  list(APPEND _example_tests usInstallCleanTest usInstallTest usExamplesCleanTest
                             usExamplesCreateDirTest)

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      add_test(NAME usExamplesConfigureTest-${config} CONFIGURATIONS ${config}
               WORKING_DIRECTORY ${_examples_binary_dir}
               COMMAND ${CMAKE_COMMAND}
                       -D CMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
                       -D CMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
                       -D BUILD_SHARED_LIBS:BOOL=${US_BUILD_SHARED_LIBS}
                       -G ${CMAKE_GENERATOR}
                       "-DCppMicroServices_DIR:PATH=${_install_dir}/${AUXILIARY_CMAKE_INSTALL_DIR}"
                       "${CMAKE_CURRENT_LIST_DIR}")
      set_tests_properties(usExamplesConfigureTest-${config} PROPERTIES
                           DEPENDS "usInstallTest;usExamplesCreateDirTest")

      add_test(NAME usExamplesBuildTest-${config} CONFIGURATIONS ${config}
               WORKING_DIRECTORY ${_examples_binary_dir}
               COMMAND ${CMAKE_COMMAND} --build . --config ${config})
      set_tests_properties(usExamplesBuildTest-${config} PROPERTIES
                           DEPENDS usExamplesConfigureTest-${config})

      list(APPEND _example_tests usExamplesConfigureTest-${config} usExamplesBuildTest-${config})
    endforeach()
  else()
    add_test(NAME usExamplesConfigureTest-${CMAKE_BUILD_TYPE}
             WORKING_DIRECTORY ${_examples_binary_dir}
             COMMAND ${CMAKE_COMMAND}
                     -D CMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                     -D CMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
                     -D CMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
                     -D BUILD_SHARED_LIBS:BOOL=${US_BUILD_SHARED_LIBS}
                     -G ${CMAKE_GENERATOR}
                     "-DCppMicroServices_DIR:PATH=${_install_dir}/${AUXILIARY_CMAKE_INSTALL_DIR}"
                     "${CMAKE_CURRENT_LIST_DIR}")
    set_tests_properties(usExamplesConfigureTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS "usInstallTest;usExamplesCreateDirTest")

    add_test(NAME usExamplesBuildTest-${CMAKE_BUILD_TYPE}
             WORKING_DIRECTORY ${_examples_binary_dir}
             COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE})
    set_tests_properties(usExamplesBuildTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS usExamplesConfigureTest-${CMAKE_BUILD_TYPE})

    list(APPEND _example_tests usExamplesConfigureTest-${CMAKE_BUILD_TYPE} usExamplesBuildTest-${CMAKE_BUILD_TYPE})
  endif()

  # The makefile is Linux specific, so only try to build the Makefile example
  # if we are on a proper system
  if(UNIX AND NOT APPLE AND US_BUILD_SHARED_LIBS)
    find_program(MAKE_COMMAND NAMES make gmake)
    find_program(CXX_COMMAND NAMES g++)
    mark_as_advanced(MAKE_COMMAND CXX_COMMAND)
    if(MAKE_COMMAND AND CXX_COMMAND)
      add_test(NAME usMakefileExampleCleanTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/makefile
               COMMAND ${MAKE_COMMAND} clean)
      add_test(NAME usMakefileExampleTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/makefile
               COMMAND ${MAKE_COMMAND})
      set_tests_properties(usMakefileExampleTest PROPERTIES
                           DEPENDS "usMakefileExampleCleanTest;usInstallTest"
                           ENVIRONMENT "CppMicroServices_ROOT=${CppMicroServices_BINARY_DIR}/install_test${CMAKE_INSTALL_PREFIX};US_CXX_FLAGS=${US_CXX_FLAGS}")
      list(APPEND _example_tests usMakefileExampleCleanTest usMakefileExampleTest)
    endif()
  endif()

  if(US_TEST_LABELS)
    set_tests_properties(${_example_tests} PROPERTIES LABELS "${US_TEST_LABELS}")
  endif()

endif()
