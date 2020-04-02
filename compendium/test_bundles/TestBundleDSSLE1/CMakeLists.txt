usFunctionCreateDSTestBundle(TestBundleDSSLE1)

usFunctionCreateTestBundleWithResources(TestBundleDSSLE1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSSLE1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
