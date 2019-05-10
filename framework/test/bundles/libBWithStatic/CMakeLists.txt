set(_tmp_shared_libs ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
usFunctionCreateTestBundleWithResources(TestBundleImportedByB
  SOURCES TestBundleImportedByB.cpp
  RESOURCES static.txt res.txt manifest.json
  RESOURCES_ROOT resources_static
  SKIP_BUNDLE_LIST)

set(BUILD_SHARED_LIBS ${_tmp_shared_libs})
usFunctionCreateTestBundleWithResources(TestBundleB
  SOURCES TestBundleB.cpp
  RESOURCES dynamic.txt res.txt manifest.json
  LINK_LIBRARIES TestBundleImportedByB)
