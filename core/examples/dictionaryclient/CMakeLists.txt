set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs)

set(dictionaryclient_DEPENDS dictionaryservice)
CreateExample(dictionaryclient ${_srcs})
