
set(snippet_src_files
  main.cpp
  SingletonOne.cpp
  SingletonTwo.cpp
)

usFunctionGenerateModuleInit(snippet_src_files
                             NAME "uServices_singleton"
                             EXECUTABLE
                            )
