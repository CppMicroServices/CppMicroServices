set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs MODULE_NAME eventlistener)

CreateExample(eventlistener ${_srcs})
