
foreach(_type ARCHIVE LIBRARY RUNTIME)
  set(CMAKE_${_type}_OUTPUT_DIRECTORY ${CMAKE_${_type}_OUTPUT_DIRECTORY}/TestModuleAL)
endforeach()

usFunctionCreateTestModuleWithResources(TestModuleAL_1 SOURCES usTestModuleAL_1.cpp RESOURCES manifest.json)
