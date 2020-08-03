usFunctionCreateDSTestBundle(TestBindUnbindThrows)

usFunctionCreateTestBundleWithResources(TestBindUnbindThrows
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME TestBindUnbindThrows
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usServiceComponent)

