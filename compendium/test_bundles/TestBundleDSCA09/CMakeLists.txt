usFunctionCreateDSTestBundle(TestBundleDSCA09)

usFunctionCreateTestBundleWithResources(TestBundleDSCA09 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA09
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)

