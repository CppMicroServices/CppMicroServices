
usFunctionCreateTestModuleWithResources(TestModuleB
  SOURCES usTestModuleB.cpp
  RESOURCES dynamic.txt res.txt)

set(BUILD_SHARED_LIBS 0)
usFunctionCreateTestModuleWithResources(TestModuleImportedByB
  SOURCES usTestModuleImportedByB.cpp
  RESOURCES static.txt res.txt
  RESOURCES_ROOT resources_static)

target_link_libraries(TestModuleB TestModuleImportedByB)
