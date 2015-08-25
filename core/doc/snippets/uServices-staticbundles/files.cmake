set(snippet_src_files
  main.cpp
)

set(base_dir uServices-staticbundles)
add_library(MyStaticBundle STATIC ${base_dir}/MyStaticBundle.cpp)
set_property(TARGET MyStaticBundle APPEND PROPERTY COMPILE_DEFINITIONS US_STATIC_BUNDLE US_BUNDLE_NAME=MyStaticBundle)
set_property(TARGET MyStaticBundle PROPERTY US_BUNDLE_NAME MyStaticBundle)

set(snippet_link_libraries
  MyStaticBundle
)
