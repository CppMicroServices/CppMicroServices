
set(resource_files
  foo.txt
  manifest.json
)

configure_file(resources/foo.txt ${CMAKE_CURRENT_BINARY_DIR}/resources/foo2.txt COPYONLY)

usFunctionCreateTestBundleWithResources(TestBundleRA
  RESOURCES ${resource_files}
  BINARY_RESOURCES foo2.txt
  APPEND_RESOURCES
)

if(APPLE)
	set_target_properties(TestBundleRA PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
endif()
