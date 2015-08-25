
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libSL1)

usFunctionCreateTestBundleWithResources(TestBundleSL4 SOURCES usActivatorSL4.cpp RESOURCES manifest.json)
