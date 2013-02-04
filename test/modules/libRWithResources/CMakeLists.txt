
set(resource_files
  icons/cppmicroservices.png
  icons/readme.txt
  foo.txt
  special_chars.dummy.txt
  test.xml
)

usFunctionCreateTestModuleWithResources(TestModuleR
  SOURCES usTestModuleR.cpp
  RESOURCES ${resource_files})

