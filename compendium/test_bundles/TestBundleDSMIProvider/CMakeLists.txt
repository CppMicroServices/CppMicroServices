usFunctionCreateDSTestBundle(TestBundleDSMIProvider)

usFunctionCreateTestBundleWithResources(TestBundleDSMIProvider
  SOURCES src/MultiInterfaceProvider.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSMIProvider
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)
