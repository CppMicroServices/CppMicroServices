usFunctionCreateDSTestBundle(DSFrenchDictionary)

usFunctionCreateTestBundleWithResources(DSFrenchDictionary USES_DS
  SOURCES src/FrenchDictionary.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSFrenchDictionary
  OTHER_LIBRARIES usTestInterfaces usIDictionaryService  usServiceComponent)
