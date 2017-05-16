
usFunctionCreateTestBundleWithResources(TestBundleU SOURCES TestBundleU.cpp RESOURCES manifest.json)

add_custom_command(TARGET TestBundleU POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:TestBundleU>"
     "$<TARGET_FILE_DIR:TestBundleU>/くいりのまちとこしくそ/$<TARGET_FILE_NAME:TestBundleU>" 
  COMMENT "Copying to output directory")

