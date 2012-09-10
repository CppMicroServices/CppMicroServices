
usFunctionCreateTestModule(TestModuleAL usTestModuleAL.cpp)

# The libAL_1 module is used in the usModuleAutoLoadTest.cpp and always
# needs to be build as a shared library
set(BUILD_SHARED_LIBS 1)
add_subdirectory(libAL_1)

