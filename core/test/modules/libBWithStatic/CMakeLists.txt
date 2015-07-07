
set(BUILD_SHARED_LIBS 0)
usFunctionCreateTestModuleWithResources(TestModuleImportedByB
  SOURCES usTestModuleImportedByB.cpp
  RESOURCES static.txt res.txt
  RESOURCES_ROOT resources_static
  SKIP_MODULE_LIST)

set(BUILD_SHARED_LIBS ${US_BUILD_SHARED_LIBS})
usFunctionCreateTestModuleWithResources(TestModuleB
  SOURCES usTestModuleB.cpp
  RESOURCES dynamic.txt res.txt
  LINK_LIBRARIES TestModuleImportedByB)
