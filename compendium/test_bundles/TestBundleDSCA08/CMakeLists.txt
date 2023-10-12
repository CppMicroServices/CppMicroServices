usFunctionCreateDSTestBundle(TestBundleDSCA08)

usFunctionCreateTestBundleWithResources(TestBundleDSCA08 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA08
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

