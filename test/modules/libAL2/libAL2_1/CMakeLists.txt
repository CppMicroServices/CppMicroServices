
foreach(_type ARCHIVE LIBRARY RUNTIME)
  set(CMAKE_${_type}_OUTPUT_DIRECTORY ${CMAKE_${_type}_OUTPUT_DIRECTORY}/autoload_al2)
endforeach()

usFunctionCreateTestModule(TestModuleAL2_1 usTestModuleAL2_1.cpp)

