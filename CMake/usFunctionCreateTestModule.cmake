
macro(_us_create_test_module_helper)

  add_library(${name} ${_srcs})
  if(NOT US_BUILD_SHARED_LIBS)
    set_property(TARGET ${name} APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_MODULE)
  endif()

  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    get_property(_compile_flags TARGET ${name} PROPERTY COMPILE_FLAGS)
    set_property(TARGET ${name} PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")
  endif()

  target_link_libraries(${name} ${US_LINK_LIBRARIES})
  if(NOT US_ENABLE_SERVICE_FACTORY_SUPPORT)
    target_link_libraries(${name} ${US_BASECLASS_LIBRARIES})
  endif()

  set(_us_test_module_libs "${_us_test_module_libs};${name}" CACHE INTERNAL "" FORCE)

endmacro()

function(usFunctionCreateTestModuleWithAutoLoadDir name autoload_dir)
  set(_srcs ${ARGN})
  usFunctionGenerateModuleInit(_srcs NAME "${name} Module" LIBRARY_NAME ${name} AUTOLOAD_DIR ${autoload_dir})
  _us_create_test_module_helper()
endfunction()

function(usFunctionCreateTestModule name)
  set(_srcs ${ARGN})
  usFunctionGenerateModuleInit(_srcs NAME "${name} Module" LIBRARY_NAME ${name})
  _us_create_test_module_helper()
endfunction()
