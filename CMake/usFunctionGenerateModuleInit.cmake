function(usFunctionGenerateModuleInit src_var)

MACRO_PARSE_ARGUMENTS(US_MODULE "NAME;LIBRARY_NAME;DEPENDS;VERSION" "EXECUTABLE" ${ARGN})

# sanity checks
if(NOT US_MODULE_NAME)
  message(SEND_ERROR "NAME argument is mandatory")
endif()

if(NOT US_MODULE_LIBRARY_NAME AND NOT US_MODULE_EXECUTABLE)
  set(US_MODULE_LIBRARY_NAME ${US_MODULE_NAME})
endif()

# Create variables of the ModuleInfo object, created in CMake/usModuleInit.cpp
set(US_MODULE_DEPENDS_STR "")
foreach(_dep ${US_MODULE_DEPENDS})
  set(US_MODULE_DEPENDS_STR "${US_MODULE_DEPENDS_STR} ${_dep}")
endforeach()

set(module_init_src_file "${CMAKE_CURRENT_BINARY_DIR}/${US_MODULE_NAME}_init.cpp")
configure_file(${CppMicroServices_SOURCE_DIR}/CMake/usModuleInit.cpp ${module_init_src_file} @ONLY)

set(_src ${${src_var}} ${module_init_src_file})
set(${src_var} ${_src} PARENT_SCOPE)

endfunction()
