
foreach(_type ARCHIVE LIBRARY RUNTIME)
  set(CMAKE_${_type}_OUTPUT_DIRECTORY ${CMAKE_${_type}_OUTPUT_DIRECTORY}/autoload_al2)
endforeach()

usFunctionCreateTestBundleWithResources(TestBundleAL2_1 SOURCES usTestBundleAL2_1.cpp RESOURCES manifest.json)
