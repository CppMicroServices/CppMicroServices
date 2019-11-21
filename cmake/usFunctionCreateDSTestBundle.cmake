function(usFunctionCreateDSTestBundle name)
  # Add in rule for how to build the autogen source for the glue

  set(_glue_file ${CMAKE_CURRENT_BINARY_DIR}/autogen_${name}_Glue.cpp)
  set(_glue_file ${_glue_file} PARENT_SCOPE)

  add_custom_command(
    OUTPUT ${_glue_file}
    COMMAND $<TARGET_FILE:SCRCodeGen> --manifest ${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json --out-file ${_glue_file} --include-headers ServiceComponents.hpp
    DEPENDS SCRCodeGen usServiceComponent ${CMAKE_CURRENT_SOURCE_DIR}/resources/manifest.json
    COMMENT "Generate bundle activator based on manifest.json"
    VERBATIM)

endfunction()
