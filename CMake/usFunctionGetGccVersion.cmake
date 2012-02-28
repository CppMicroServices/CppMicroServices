
#! \brief Get the gcc version
FUNCTION(usFunctionGetGccVersion path_to_gcc output_var)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    EXECUTE_PROCESS(
      COMMAND ${path_to_gcc} -dumpversion
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE error
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
      )
    IF(result)
      MESSAGE(FATAL_ERROR "Failed to obtain compiler version running [${path_to_gcc} -dumpversion]: ${error}")
    ENDIF()
    SET(${output_var} ${output} PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()

