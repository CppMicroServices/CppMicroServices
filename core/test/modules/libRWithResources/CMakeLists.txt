
set(resource_files
  icons/compressable.bmp
  icons/cppmicroservices.png
  icons/readme.txt
  foo.txt
  special_chars.dummy.txt
  test.xml
)

configure_file(resources/foo.txt ${CMAKE_CURRENT_BINARY_DIR}/foo2.txt @COPYONLY)

#usFunctionCreateTestModuleWithResources(TestModuleR
#  SOURCES usTestModuleR.cpp
#  RESOURCES ${resource_files})

usFunctionCreateTestModule(TestModuleR)
usFunctionAddResources(
  TARGET TestModuleR
  WORKING_DIRECTORY resources
  FILES ${resource_files}
  )
