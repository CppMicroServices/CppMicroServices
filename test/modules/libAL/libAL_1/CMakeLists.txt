
foreach(_type ARCHIVE LIBRARY RUNTIME)
  set(CMAKE_${_type}_OUTPUT_DIRECTORY ${CMAKE_${_type}_OUTPUT_DIRECTORY}/TestModuleAL)
endforeach()

usFunctionCreateTestModule(TestModuleAL_1 usTestModuleAL_1.cpp)

