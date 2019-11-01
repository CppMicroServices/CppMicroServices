set(_srcs
  ComponentConstants.cpp
  ComponentContext.cpp
  ComponentException.cpp
  ComponentInstance.cpp
  ServiceComponentRuntime.cpp
  )

set(_public_headers
  ../include/cppmicroservices/servicecomponent/ComponentConstants.hpp
  ../include/cppmicroservices/servicecomponent/ComponentContext.hpp
  ../include/cppmicroservices/servicecomponent/ComponentException.hpp
  ../include/cppmicroservices/servicecomponent/detail/Binders.hpp
  ../include/cppmicroservices/servicecomponent/detail/ComponentInstance.hpp
  ../include/cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp
  ../include/cppmicroservices/servicecomponent/runtime/ServiceComponentRuntime.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/BundleDTO.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/ComponentConfigurationDTO.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/ComponentDescriptionDTO.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/ReferenceDTO.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/SatisfiedReferenceDTO.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/ServiceReferenceDTO.hpp
  ../include/cppmicroservices/servicecomponent/runtime/dto/UnsatisfiedReferenceDTO.hpp
  )
  
# Compile definitions
add_compile_definitions(usServiceComponent_EXPORTS)
  
add_library(ServiceComponentObjs OBJECT ${_srcs} ${_public_headers})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  get_property(_compile_flags TARGET ServiceComponentObjs PROPERTY COMPILE_FLAGS)
  set_property(TARGET ServiceComponentObjs PROPERTY COMPILE_FLAGS "${_compile_flags} -fPIC")
endif()

include_directories(
  ${CppMicroServices_SOURCE_DIR}/framework/include
  ${CppMicroServices_SOURCE_DIR}/compendium/ServiceComponent/include
  ${CppMicroServices_BINARY_DIR}/include
  ${CppMicroServices_BINARY_DIR}/framework/include
  ${CppMicroServices_BINARY_DIR}/compendium/ServiceComponent/include
  )

