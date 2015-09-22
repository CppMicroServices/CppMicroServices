set(_srcs Activator.cpp)

set(dictionaryclient_DEPENDS dictionaryservice)
CreateExample(dictionaryclient ${_srcs})
