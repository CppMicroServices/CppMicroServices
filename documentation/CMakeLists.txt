
if(US_BUILD_TESTING)
  include(usFunctionCompileSnippets)
  # Compile source code snippets
  add_subdirectory(snippets)
endif()

if(NOT US_IS_EMBEDDED)
  find_package(Doxygen)

  if(DOXYGEN_FOUND)

    set(US_HAVE_DOT "NO")
    if(DOXYGEN_DOT_EXECUTABLE)
      set(US_HAVE_DOT "YES")
    endif()

    if(NOT DEFINED US_DOXYGEN_DOT_NUM_THREADS)
      set(US_DOXYGEN_DOT_NUM_THREADS 4)
    endif()
    
    if(US_IS_EMBEDDED)
      set(US_DOXYGEN_MAIN_PAGE_CMD "\\page \\1")
    else()
      set(US_DOXYGEN_MAIN_PAGE_CMD "\\mainpage")
    endif()

    configure_file(doxygen.conf.in
                   ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

    add_custom_target(doc
                      ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      )
  endif()
endif()

