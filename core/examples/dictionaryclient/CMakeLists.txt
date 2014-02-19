set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "Dictionary Client"
                             LIBRARY_NAME "dictionaryclient")

set(dictionaryclient_DEPENDS dictionaryservice)
CreateExample(dictionaryclient ${_srcs})
