set(_srcs Activator.cpp)

usFunctionGenerateBundleInit(_srcs)

set(dictionaryclient_DEPENDS dictionaryservice)
CreateExample(dictionaryclient ${_srcs})
