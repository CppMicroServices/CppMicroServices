set(_srcs Activator.cpp)

usFunctionGenerateBundleInit(_srcs)

set(dictionaryclient3_DEPENDS dictionaryservice)
CreateExample(dictionaryclient3 ${_srcs})
