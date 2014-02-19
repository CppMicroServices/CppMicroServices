set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "French Dictionary"
                             LIBRARY_NAME "frenchdictionary")

set(frenchdictionary_DEPENDS dictionaryservice)
CreateExample(frenchdictionary ${_srcs})
