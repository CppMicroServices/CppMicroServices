CXX = g++
CXXFLAGS = -g -Wall -Wno-unused -pedantic -fPIC $(US_CXX_FLAGS)
LDFLAGS = -Wl,-rpath="$(CppMicroServices_ROOT)/lib" -Wl,-rpath=.
LDLIBS = -lCppMicroServices${US_DEBUG_POSTFIX}

INCLUDEDIRS = -I"$(CppMicroServices_ROOT)/include/cppmicroservices3"
LIBDIRS = -L"$(CppMicroServices_ROOT)/lib" -L.

all : main libbundle.so

main: libbundle.so main.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(INCLUDEDIRS) $(LIBDIRS) $(LDLIBS) -lbundle

libbundle.so: bundle.o IDictionaryService.o
	$(CXX) -shared -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(INCLUDEDIRS) $(LIBDIRS) $(LDLIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -DUS_BUNDLE_NAME=main $(INCLUDEDIRS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -DMODULE_EXPORTS -DUS_BUNDLE_NAME=bundle $(INCLUDEDIRS) -c $< -o $@

.PHONY : clean

clean:
	rm -f *.o
