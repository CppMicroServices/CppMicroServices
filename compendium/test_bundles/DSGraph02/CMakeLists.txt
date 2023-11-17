usFunctionCreateDSTestBundle(DSGraph02)

usFunctionCreateTestBundleWithResources(DSGraph02
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph02
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
