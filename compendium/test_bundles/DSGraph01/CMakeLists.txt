usFunctionCreateDSTestBundle(DSGraph01)

usFunctionCreateTestBundleWithResources(DSGraph01 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph01
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
