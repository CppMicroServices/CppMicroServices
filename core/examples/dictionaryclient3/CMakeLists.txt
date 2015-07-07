set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs)

set(dictionaryclient3_DEPENDS dictionaryservice)
CreateExample(dictionaryclient3 ${_srcs})
