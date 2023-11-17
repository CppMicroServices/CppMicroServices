usFunctionCreateDSTestBundle(TestBundleDSSLE2)

usFunctionCreateTestBundleWithResources(TestBundleDSSLE2
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSSLE2
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
