project(FrameworkExamples)

find_package(CppMicroServices NO_MODULE REQUIRED)

cmake_minimum_required(VERSION ${US_CMAKE_MINIMUM_REQUIRED_VERSION})
cmake_policy(VERSION ${US_CMAKE_MINIMUM_REQUIRED_VERSION})

include_directories(${CppMicroServices_INCLUDE_DIRS})

#-----------------------------------------------------------------------------
# Set C/CXX flags
#-----------------------------------------------------------------------------

if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${US_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${US_CXX_FLAGS_RELEASE}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${US_CXX_FLAGS_DEBUG}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${US_C_FLAGS}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${US_C_FLAGS_RELEASE}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${US_C_FLAGS_DEBUG}")
endif()

#-----------------------------------------------------------------------------
# Init output directories
#-----------------------------------------------------------------------------

set(FrameworkExamples_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(FrameworkExamples_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(FrameworkExamples_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

foreach(_type ARCHIVE LIBRARY RUNTIME)
  if(NOT CMAKE_${_type}_OUTPUT_DIRECTORY)
    set(CMAKE_${_type}_OUTPUT_DIRECTORY ${FrameworkExamples_${_type}_OUTPUT_DIRECTORY})
  endif()
endforeach()


function(CreateExample _name)
  set(_srcs ${ARGN})

  usFunctionGetResourceSource(TARGET Example-${_name} OUT _srcs)
  usFunctionGenerateBundleInit(_srcs)

  add_library(Example-${_name} ${_srcs})
  set_property(TARGET Example-${_name} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=${_name})
  set_property(TARGET Example-${_name} PROPERTY DEBUG_POSTFIX "")

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

  usFunctionAddResources(TARGET Example-${_name} BUNDLE_NAME ${_name} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/${_name}/resources FILES manifest.json)
  usFunctionEmbedResources(TARGET Example-${_name})

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

  add_test(NAME InstallCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${_install_dir}")

  add_test(NAME InstallTest
           WORKING_DIRECTORY ${CppMicroServices_BINARY_DIR}
           COMMAND ${CMAKE_COMMAND} --build ${CppMicroServices_BINARY_DIR} --config $<CONFIGURATION> --target install)
  set_tests_properties(InstallTest PROPERTIES
                       ENVIRONMENT "DESTDIR=${CppMicroServices_BINARY_DIR}/install_test"
                       DEPENDS InstallCleanTest)

  set(_examples_binary_dir "${CppMicroServices_BINARY_DIR}/examples_build")

  add_test(NAME ExamplesCleanTest
           COMMAND ${CMAKE_COMMAND} -E remove_directory "${_examples_binary_dir}")

  add_test(NAME ExamplesCreateDirTest
           COMMAND ${CMAKE_COMMAND} -E make_directory "${_examples_binary_dir}")
  set_tests_properties(ExamplesCreateDirTest PROPERTIES
                       DEPENDS ExamplesCleanTest)

  list(APPEND _example_tests InstallCleanTest InstallTest ExamplesCleanTest
                             ExamplesCreateDirTest)

  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      add_test(NAME ExamplesConfigureTest-${config} CONFIGURATIONS ${config}
               WORKING_DIRECTORY ${_examples_binary_dir}
               COMMAND ${CMAKE_COMMAND}
                       -D CMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
                       -D CMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
                       -D US_BUILD_SHARED_LIBS:BOOL=${US_BUILD_SHARED_LIBS}
                       -G ${CMAKE_GENERATOR}
                       "-DCppMicroServices_DIR:PATH=${_install_dir}/${AUXILIARY_CMAKE_INSTALL_DIR}"
                       "${CMAKE_CURRENT_LIST_DIR}")
      set_tests_properties(ExamplesConfigureTest-${config} PROPERTIES
                           DEPENDS "InstallTest;ExamplesCreateDirTest")

      add_test(NAME ExamplesBuildTest-${config} CONFIGURATIONS ${config}
               WORKING_DIRECTORY ${_examples_binary_dir}
               COMMAND ${CMAKE_COMMAND} --build . --config ${config})
      set_tests_properties(ExamplesBuildTest-${config} PROPERTIES
                           DEPENDS ExamplesConfigureTest-${config})

      list(APPEND _example_tests ExamplesConfigureTest-${config} ExamplesBuildTest-${config})
    endforeach()
  else()
    add_test(NAME ExamplesConfigureTest-${CMAKE_BUILD_TYPE}
             WORKING_DIRECTORY ${_examples_binary_dir}
             COMMAND ${CMAKE_COMMAND}
                     -D CMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                     -D CMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
                     -D CMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
                     -D US_BUILD_SHARED_LIBS:BOOL=${US_BUILD_SHARED_LIBS}
                     -G ${CMAKE_GENERATOR}
                     "-DCppMicroServices_DIR:PATH=${_install_dir}/${AUXILIARY_CMAKE_INSTALL_DIR}"
                     "${CMAKE_CURRENT_LIST_DIR}")
    set_tests_properties(ExamplesConfigureTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS "InstallTest;ExamplesCreateDirTest")

    add_test(NAME ExamplesBuildTest-${CMAKE_BUILD_TYPE}
             WORKING_DIRECTORY ${_examples_binary_dir}
             COMMAND ${CMAKE_COMMAND} --build . --config ${CMAKE_BUILD_TYPE})
    set_tests_properties(ExamplesBuildTest-${CMAKE_BUILD_TYPE} PROPERTIES
                         DEPENDS ExamplesConfigureTest-${CMAKE_BUILD_TYPE})

    list(APPEND _example_tests ExamplesConfigureTest-${CMAKE_BUILD_TYPE} ExamplesBuildTest-${CMAKE_BUILD_TYPE})
  endif()

  # The makefile is Linux specific, so only try to build the Makefile example
  # if we are on a proper system
  if(UNIX AND NOT APPLE AND US_BUILD_SHARED_LIBS)
    find_program(MAKE_COMMAND NAMES make gmake)
    find_program(CXX_COMMAND NAMES g++)
    mark_as_advanced(MAKE_COMMAND CXX_COMMAND)
    if(MAKE_COMMAND AND CXX_COMMAND)
      add_test(NAME MakefileExampleCleanTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/makefile
               COMMAND ${MAKE_COMMAND} clean)
      add_test(NAME MakefileExampleTest
               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/makefile
               COMMAND ${MAKE_COMMAND})
      set_tests_properties(MakefileExampleTest PROPERTIES
                           DEPENDS "MakefileExampleCleanTest;InstallTest"
                           ENVIRONMENT "CppMicroServices_ROOT=${CppMicroServices_BINARY_DIR}/install_test${CMAKE_INSTALL_PREFIX};US_CXX_FLAGS=${US_CXX_FLAGS};US_DEBUG_POSTFIX=${CMAKE_DEBUG_POSTFIX}")
      list(APPEND _example_tests MakefileExampleCleanTest MakefileExampleTest)
    endif()
  endif()

  set_tests_properties(${_example_tests} PROPERTIES
                       LABELS regular)

endif()
