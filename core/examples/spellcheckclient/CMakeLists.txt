set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs)

set(spellcheckclient_DEPENDS spellcheckservice)
CreateExample(spellcheckclient ${_srcs})
