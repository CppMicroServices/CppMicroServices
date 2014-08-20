set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs MODULE_NAME dictionaryclient)

set(dictionaryclient_DEPENDS dictionaryservice)
CreateExample(dictionaryclient ${_srcs})
