set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs MODULE_NAME frenchdictionary)

set(frenchdictionary_DEPENDS dictionaryservice)
CreateExample(frenchdictionary ${_srcs})
