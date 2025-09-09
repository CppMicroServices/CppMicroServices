add_library(util OBJECT
  include/cppmicroservices/util/BundleElfFile.h
  include/cppmicroservices/util/BundleMachOFile.h
  include/cppmicroservices/util/BundleObjFactory.h
  include/cppmicroservices/util/BundleObjFile.h
  include/cppmicroservices/util/BundlePEFile.h
  include/cppmicroservices/util/DataContainer.h
  include/cppmicroservices/util/Error.h
  include/cppmicroservices/util/FileSystem.h
  include/cppmicroservices/util/MappedFile.h
  include/cppmicroservices/util/String.h
  include/cppmicroservices/util/StringReplace.h
  
  src/BundleObjFactory.cpp
  src/BundleObjFile.cpp
  src/Error.cpp
  src/FileSystem.cpp
  src/String.cpp
)

set_property(TARGET util PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(util
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CppMicroServices_BINARY_DIR}/framework/include
    ${CppMicroServices_SOURCE_DIR}/framework/include
    ${CppMicroServices_SOURCE_DIR}/compendium/LogService/include
    ${CppMicroServices_BINARY_DIR}/compendium/LogService/include
 
  PRIVATE
    ${PROJECT_SOURCE_DIR}/third_party
    ${PROJECT_BINARY_DIR}/include
)

add_dependencies(util usLogService)
