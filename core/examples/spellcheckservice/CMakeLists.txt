set(_srcs Activator.cpp ISpellCheckService.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "Spell Check Service"
                             LIBRARY_NAME "spellcheckservice")

set(spellcheckservice_DEPENDS dictionaryservice)
CreateExample(spellcheckservice ${_srcs})
