set(_srcs Activator.cpp ISpellCheckService.cpp)

usFunctionGenerateBundleInit(_srcs)

set(spellcheckservice_DEPENDS dictionaryservice)
CreateExample(spellcheckservice ${_srcs})
