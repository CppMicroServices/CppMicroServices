usFunctionCreateTestBundleWithResources(EnglishDictionary
  SOURCES src/EnglishDictionary.cpp src/Activator.cpp
  RESOURCES manifest.json
  BUNDLE_SYMBOLIC_NAME EnglishDictionary
  OTHER_LIBRARIES usTestInterfaces usIDictionaryService)
