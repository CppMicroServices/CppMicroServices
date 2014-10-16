set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs)

set(frenchdictionary_DEPENDS dictionaryservice)
CreateExample(frenchdictionary ${_srcs})
