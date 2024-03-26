add_custom_target(rc_tests_build
                  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test/basic_test.sh ${CMAKE_CURRENT_BINARY_DIR}/basic_test.sh
                  DEPENDS test/basic_test.sh)

add_dependencies(${US_RCC_EXECUTABLE_TARGET} rc_tests_build)

if (MSVC)
  set(SHA_SUM_FOR_TEST "sha512sum.exe -b")
else()
  set(SHA_SUM_FOR_TEST "shasum -b -a 512")
endif ()

if (US_USE_DETERMINISTIC_BUNDLE_BUILDS)
  add_test(NAME Deterministic_Zip_Files_Test
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND env SHA_SUM_CMD=${SHA_SUM_FOR_TEST} RC_EXE=${CMAKE_BINARY_DIR}/bin/${US_RCC_EXECUTABLE_OUTPUT_NAME} ./basic_test.sh
         )
endif()

