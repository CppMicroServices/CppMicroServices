#
# Helper macro allowing to return path to boost library
#
# Usage:
#    usFunctionBoostPath(USE_SYSTEM ${US_USE_SYSTEM_BOOST} CPPMS_SOURCE_DIR ${CppMicroServices_SOURCE_DIR}
#      BOOST_DIR ${BOOST_INCLUDEDIR})


function(usFunctionBoostPath)
  cmake_parse_arguments(Boost_Path "" "BOOST_SYSTEM;CPPMS_SOURCE_DIR;BOOST_DIR" "" ${ARGN})

  if (Boost_Path_BOOST_SYSTEM)
      set(_boost_library ${Boost_Path_BOOST_DIR} PARENT_SCOPE)
  else()
      set(_boost_library ${Boost_Path_CPPMS_SOURCE_DIR}/third_party/boost/include PARENT_SCOPE)
  endif()
endfunction()
