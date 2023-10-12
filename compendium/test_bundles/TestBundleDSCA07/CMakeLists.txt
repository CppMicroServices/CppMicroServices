usFunctionCreateDSTestBundle(TestBundleDSCA07)

usFunctionCreateTestBundleWithResources(TestBundleDSCA07 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBundleDSCA07
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

