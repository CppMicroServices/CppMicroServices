set(_srcs Activator.cpp ISpellCheckService.cpp)

set(spellcheckservice_DEPENDS dictionaryservice)
CreateExample(spellcheckservice ${_srcs})
