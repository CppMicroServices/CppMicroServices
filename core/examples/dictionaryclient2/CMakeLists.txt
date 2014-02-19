set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "Dictionary Client 2"
                             LIBRARY_NAME "dictionaryclient2")

set(dictionaryclient2_DEPENDS dictionaryservice)
CreateExample(dictionaryclient2 ${_srcs})
