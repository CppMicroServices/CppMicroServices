set(_srcs Activator.cpp)

usFunctionGenerateBundleInit(_srcs)

set(frenchdictionary_DEPENDS dictionaryservice)
CreateExample(frenchdictionary ${_srcs})
