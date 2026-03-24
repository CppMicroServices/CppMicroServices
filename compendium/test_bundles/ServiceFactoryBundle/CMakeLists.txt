usFunctionCreateDSTestBundle(ServiceFactoryBundle)

usFunctionCreateTestBundleWithResources(ServiceFactoryBundle
  SOURCES ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME ServiceFactoryBundle
  OTHER_LIBRARIES usTestInterfaces usServiceComponent cm)
