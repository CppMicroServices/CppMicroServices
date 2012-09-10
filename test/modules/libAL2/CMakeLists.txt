
usFunctionCreateTestModuleWithAutoLoadDir(TestModuleAL2 "autoload_al2" usTestModuleAL2.cpp)

# The libAL2_1 module is used in the usModuleAutoLoadTest.cpp and always
# needs to be build as a shared library
set(BUILD_SHARED_LIBS 1)
add_subdirectory(libAL2_1)

