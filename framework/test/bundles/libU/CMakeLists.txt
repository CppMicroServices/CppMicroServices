
usFunctionCreateTestBundleWithResources(TestBundleU SOURCES TestBundleU.cpp RESOURCES manifest.json LINK_RESOURCES)

# override the library and runtime output directories for the target to include unicode characters in path
set(unicode_dir_name くいりのまちとこしくそ)
if(DEFINED CMAKE_CONFIGURATION_TYPES)   # necessary for generators such as Visual Studio and Xcode
  foreach(_type ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${_type} cur_config)
    set_target_properties(TestBundleU PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${cur_config} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_type}/${unicode_dir_name}/ )
    set_target_properties(TestBundleU PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${cur_config} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_type}/${unicode_dir_name}/ )
  endforeach()
endif()
set_target_properties(TestBundleU PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${unicode_dir_name}/ )
set_target_properties(TestBundleU PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${unicode_dir_name}/ )

