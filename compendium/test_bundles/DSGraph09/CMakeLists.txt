usFunctionCreateDSTestBundle(DSGraph09)

usFunctionCreateTestBundleWithResources(DSGraph09
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSGraph09
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
