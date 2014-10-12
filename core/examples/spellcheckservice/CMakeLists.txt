set(_srcs Activator.cpp ISpellCheckService.cpp)

usFunctionGenerateModuleInit(_srcs)

set(spellcheckservice_DEPENDS dictionaryservice)
CreateExample(spellcheckservice ${_srcs})
