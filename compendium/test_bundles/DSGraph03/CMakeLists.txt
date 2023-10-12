usFunctionCreateDSTestBundle(DSGraph03)

usFunctionCreateTestBundleWithResources(DSGraph03 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph03
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
