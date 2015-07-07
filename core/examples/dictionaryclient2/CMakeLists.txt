set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs)

set(dictionaryclient2_DEPENDS dictionaryservice)
CreateExample(dictionaryclient2 ${_srcs})
