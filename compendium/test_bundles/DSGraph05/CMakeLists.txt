usFunctionCreateDSTestBundle(DSGraph05)

usFunctionCreateTestBundleWithResources(DSGraph05
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph05
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
