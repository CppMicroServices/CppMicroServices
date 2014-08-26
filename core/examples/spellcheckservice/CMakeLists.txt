set(_srcs Activator.cpp ISpellCheckService.cpp)

usFunctionGenerateModuleInit(_srcs MODULE_NAME spellcheckservice)

set(spellcheckservice_DEPENDS dictionaryservice)
CreateExample(spellcheckservice ${_srcs})
