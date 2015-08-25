
foreach(_type ARCHIVE LIBRARY RUNTIME)
  set(CMAKE_${_type}_OUTPUT_DIRECTORY ${CMAKE_${_type}_OUTPUT_DIRECTORY}/TestBundleAL)
endforeach()

usFunctionCreateTestBundleWithResources(TestBundleAL_1 SOURCES usTestBundleAL_1.cpp RESOURCES manifest.json)
