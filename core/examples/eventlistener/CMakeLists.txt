set(_srcs Activator.cpp)

usFunctionGenerateModuleInit(_srcs
                             NAME "Event Listener"
                             LIBRARY_NAME "eventlistener")

CreateExample(eventlistener ${_srcs})
