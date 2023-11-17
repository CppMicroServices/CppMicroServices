usFunctionCreateDSTestBundle(TestBundleDSTOI22)

usFunctionCreateTestBundleWithResources(TestBundleDSTOI22
  SOURCES src/Activator.cpp src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSTOI22
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

