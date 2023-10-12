usFunctionCreateDSTestBundle(DSGraph05)

usFunctionCreateTestBundleWithResources(DSGraph05 USES_DS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph05
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
