usFunctionCreateDSTestBundle(TestBundleDSCAI1)

usFunctionCreateTestBundleWithResources(TestBundleDSCAI1
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCAI1
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

