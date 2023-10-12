usFunctionCreateDSTestBundle(TestBundleDSDependentNoInject)

usFunctionCreateTestBundleWithResources(TestBundleDSDependentNoInject USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSDependentNoInject
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
