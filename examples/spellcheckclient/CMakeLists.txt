set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "Spell Check Client"
                             LIBRARY_NAME "spellcheckclient")

set(spellcheckclient_DEPENDS spellcheckservice)
CreateExample(spellcheckclient ${_srcs})
