set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "Dictionary Client 3"
                             LIBRARY_NAME "dictionaryclient3")

set(dictionaryclient3_DEPENDS dictionaryservice)
CreateExample(dictionaryclient3 ${_srcs})
