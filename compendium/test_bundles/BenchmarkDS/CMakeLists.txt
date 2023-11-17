usFunctionCreateDSTestBundle(BenchmarkDS)

usFunctionCreateTestBundleWithResources(BenchmarkDS
  SOURCES src/ServiceImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME BenchmarkDS
  OTHER_LIBRARIES usTestInterfaces usServiceComponent)
