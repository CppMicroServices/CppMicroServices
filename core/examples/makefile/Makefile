CXX = g++
CXXFLAGS = -g -Wall -Wno-unused -pedantic -fPIC $(US_CXX_FLAGS)
LDFLAGS = -Wl,-rpath="$(CppMicroServices_ROOT)/lib" -Wl,-rpath=.
LDLIBS = -lCppMicroServices

INCLUDEDIRS = -I"$(CppMicroServices_ROOT)/include/CppMicroServices"
LIBDIRS = -L"$(CppMicroServices_ROOT)/lib/CppMicroServices" -L.

all : main libmodule.so

main: libmodule.so main.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(INCLUDEDIRS) $(LIBDIRS) $(LDLIBS) -lmodule

libmodule.so: module.o IDictionaryService.o
	$(CXX) -shared -o $@ $^ $(CXXFLAGS) $(LDFLAGS) $(INCLUDEDIRS) $(LIBDIRS) $(LDLIBS)

main.o: main.cpp
	$(CXX) $(CXXFLAGS) -DUS_MODULE_NAME=main $(INCLUDEDIRS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -DMODULE_EXPORTS -DUS_MODULE_NAME=module $(INCLUDEDIRS) -c $< -o $@

.PHONY : clean

clean:
	rm -f *.o
