
set(resource_files
  icons/compressable.bmp
  icons/cppmicroservices.png
  icons/readme.txt
  foo.txt
  special_chars.dummy.txt
  test.xml
)

configure_file(resources/foo.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/foo2.txt @COPYONLY)

usFunctionCreateTestModuleWithResources(TestModuleR
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/TestModuleR_binary_resources.cpp
  RESOURCES ${resource_files}
)

usFunctionAddResources(
  TARGET TestModuleR
  SOURCE_OUTPUT TestModuleR_binary_resources.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
  FILES foo2.txt
)
