usFunctionCreateDSTestBundle(DSGraph06)

usFunctionCreateTestBundleWithResources(DSGraph06 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph06
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
