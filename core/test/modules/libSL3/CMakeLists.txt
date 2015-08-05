
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libSL1)

usFunctionCreateTestModuleWithResources(TestModuleSL3 SOURCES usActivatorSL3.cpp RESOURCES manifest.json)
