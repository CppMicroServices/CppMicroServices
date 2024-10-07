usFunctionCreateDSTestBundle(GloballyNamespacedInterfaces)

usFunctionCreateTestBundleWithResources(GloballyNamespacedInterfaces
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME GloballyNamespacedInterfaces
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
