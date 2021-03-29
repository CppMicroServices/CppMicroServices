
set(resource_files
  icons/compressable.bmp
  icons/cppmicroservices.png
  icons/readme.txt
  foo.ptxt
  special_chars.dummy.ptxt
  test.xml
  manifest.json
)

configure_file(resources/foo.ptxt ${CMAKE_CURRENT_BINARY_DIR}/resources/foo2.ptxt COPYONLY)

usFunctionCreateTestBundleWithResources(TestBundleR
  RESOURCES ${resource_files}
  BINARY_RESOURCES foo2.ptxt
)
