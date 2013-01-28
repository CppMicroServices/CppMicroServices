#
# Helper macro allowing to check if the given flags are supported
# by the underlying build tool
#
# If the flag(s) is/are supported, they will be appended to the string identified by RESULT_VAR
#
# Usage:
#   usFunctionCheckCompilerFlags(FLAGS_TO_CHECK VALID_FLAGS_VAR)
#
# Example:
#
#   set(myflags)
#   usFunctionCheckCompilerFlags("-fprofile-arcs" myflags)
#   message(1-myflags:${myflags})
#   usFunctionCheckCompilerFlags("-fauto-bugfix" myflags)
#   message(2-myflags:${myflags})
#   usFunctionCheckCompilerFlags("-Wall" myflags)
#   message(1-myflags:${myflags})
#
#   The output will be:
#    1-myflags: -fprofile-arcs
#    2-myflags: -fprofile-arcs
#    3-myflags: -fprofile-arcs -Wall

include(TestCXXAcceptsFlag)

function(usFunctionCheckCompilerFlags CXX_FLAG_TO_TEST RESULT_VAR)

  if(CXX_FLAG_TO_TEST STREQUAL "")
    message(FATAL_ERROR "CXX_FLAG_TO_TEST shouldn't be empty")
  endif()

  set(_test_flag ${CXX_FLAG_TO_TEST})
  CHECK_CXX_ACCEPTS_FLAG("-Werror=unknown-warning-option" HAS_FLAG_unknown-warning-option)
  if(HAS_FLAG_unknown-warning-option)
    set(_test_flag "-Werror=unknown-warning-option ${CXX_FLAG_TO_TEST}")
  endif()

  # Internally, the macro CMAKE_CXX_ACCEPTS_FLAG calls TRY_COMPILE. To avoid
  # the cost of compiling the test each time the project is configured, the variable set by
  # the macro is added to the cache so that following invocation of the macro with
  # the same variable name skip the compilation step.
  # For that same reason, ctkFunctionCheckCompilerFlags function appends a unique suffix to
  # the HAS_FLAG variable. This suffix is created using a 'clean version' of the flag to test.
  string(REGEX REPLACE "-\\s\\$\\+\\*\\{\\}\\(\\)\\#" "" suffix ${CXX_FLAG_TO_TEST})
  CHECK_CXX_ACCEPTS_FLAG(${_test_flag} HAS_FLAG_${suffix})

  if(HAS_FLAG_${suffix})
    set(${RESULT_VAR} "${${RESULT_VAR}} ${CXX_FLAG_TO_TEST}" PARENT_SCOPE)
  endif()

endfunction()

