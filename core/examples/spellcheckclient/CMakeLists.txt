set(_srcs Activator.cpp)

set(spellcheckclient_DEPENDS spellcheckservice)
CreateExample(spellcheckclient ${_srcs})
