usFunctionCreateDSTestBundle(TestBundleDSMIConsumer)

usFunctionCreateTestBundleWithResources(TestBundleDSMIConsumer
  SOURCES src/MultiInterfaceConsumer.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSMIConsumer
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)
