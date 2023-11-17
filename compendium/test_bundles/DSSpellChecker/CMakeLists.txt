usFunctionCreateDSTestBundle(DSSpellChecker)

usFunctionCreateTestBundleWithResources(DSSpellChecker
  SOURCES src/SpellCheckImpl.cpp ${_glue_file}
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME DSSpellChecker
  OTHER_LIBRARIES usTestInterfaces usServiceComponent usIDictionaryService usISpellCheckService)

