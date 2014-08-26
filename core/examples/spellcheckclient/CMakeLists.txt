set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs MODULE_NAME spellcheckclient)

set(spellcheckclient_DEPENDS spellcheckservice)
CreateExample(spellcheckclient ${_srcs})
