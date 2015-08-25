set(_srcs Activator.cpp)

usFunctionGenerateBundleInit(_srcs)

CreateExample(eventlistener ${_srcs})
