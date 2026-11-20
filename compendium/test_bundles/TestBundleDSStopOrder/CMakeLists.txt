usFunctionCreateDSTestBundle(TestBundleDSStopOrder)

usFunctionCreateTestBundleWithResources(TestBundleDSStopOrder
  SOURCES src/Activator.cpp src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSStopOrder
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
