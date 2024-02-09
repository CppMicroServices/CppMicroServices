usFunctionCreateDSTestBundle(ManagedFactoryCMTest)

usFunctionCreateTestBundleWithResources(ManagedFactoryCMTest
  SOURCES src/Factory.cpp src/FactoryServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME ManagedFactoryCMTest
  OTHER_LIBRARIES usTestInterfaces usServiceComponent cm)
