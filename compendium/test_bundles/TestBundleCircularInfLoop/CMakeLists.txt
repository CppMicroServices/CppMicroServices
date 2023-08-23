usFunctionCreateDSTestBundle(TestBundleCircularInfLoop)

usFunctionCreateTestBundleWithResources(TestBundleCircularInfLoop
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleCircularInfLoop
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

