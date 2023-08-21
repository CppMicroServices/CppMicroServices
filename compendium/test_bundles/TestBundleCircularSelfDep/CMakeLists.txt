usFunctionCreateDSTestBundle(TestBundleCircularSelfDep)

usFunctionCreateTestBundleWithResources(TestBundleCircularSelfDep
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularSelfDep
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

