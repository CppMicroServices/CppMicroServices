usFunctionCreateDSTestBundle(DSGraph07)

usFunctionCreateTestBundleWithResources(DSGraph07 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph07
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
