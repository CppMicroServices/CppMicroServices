set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs MODULE_NAME dictionaryclient3)

set(dictionaryclient3_DEPENDS dictionaryservice)
CreateExample(dictionaryclient3 ${_srcs})
