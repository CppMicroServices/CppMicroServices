
function(usFunctionCreateTestModule name)

  set(_srcs ${ARGN})
  usFunctionGenerateModuleInit(_srcs NAME ${name})

  add_library(${name} ${_srcs})
  target_link_libraries(${name} ${US_LINK_LIBRARIES})
  if(NOT US_ENABLE_SERVICE_FACTORY_SUPPORT)
    target_link_libraries(${name} ${US_BASECLASS_LIBRARIES})
  endif()

  set(_us_test_module_libs "${_us_test_module_libs};${name}" CACHE INTERNAL "" FORCE)
  
endfunction()
