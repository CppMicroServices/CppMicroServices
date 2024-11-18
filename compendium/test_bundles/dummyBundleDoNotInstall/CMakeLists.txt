usFunctionCreateDSTestBundle(dummyBundleDoNotInstall)

usFunctionCreateTestBundleWithResources(dummyBundleDoNotInstall
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME dummyBundleDoNotInstall
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
