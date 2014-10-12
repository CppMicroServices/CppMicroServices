set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs)

CreateExample(eventlistener ${_srcs})
