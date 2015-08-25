set(_srcs Activator.cpp)

usFunctionGenerateBundleInit(_srcs)

set(spellcheckclient_DEPENDS spellcheckservice)
CreateExample(spellcheckclient ${_srcs})
