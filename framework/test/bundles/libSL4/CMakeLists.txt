
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libSL1)

FunctionCreateTestBundleWithResources(TestBundleSL4 SOURCES ActivatorSL4.cpp RESOURCES manifest.json)
