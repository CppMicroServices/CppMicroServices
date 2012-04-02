
if(US_BUILD_TESTING)
  include(usFunctionCompileSnippets)
  # Compile source code snippets
  add_subdirectory(snippets)
endif()

if(NOT US_IS_EMBEDDED)
  find_package(Doxygen)

  if(DOXYGEN_FOUND)
  
    option(US_DOCUMENTATION_FOR_WEBPAGE "Build Doxygen documentation for the webpage" OFF)
    set(US_DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE PATH "Doxygen output directory")
    mark_as_advanced(US_DOCUMENTATION_FOR_WEBPAGE US_DOXYGEN_OUTPUT_DIR)

    set(US_HAVE_DOT "NO")
    if(DOXYGEN_DOT_EXECUTABLE)
      set(US_HAVE_DOT "YES")
    endif()

    if(NOT DEFINED US_DOXYGEN_DOT_NUM_THREADS)
      set(US_DOXYGEN_DOT_NUM_THREADS 4)
    endif()
    
    # We are in standalone mode, so we generate a "mainpage"
    set(US_DOXYGEN_MAIN_PAGE_CMD "\\mainpage")
    
    if(US_DOCUMENTATION_FOR_WEBPAGE)
      configure_file(doxygen/header.html
                     ${CMAKE_CURRENT_BINARY_DIR}/header.html COPY_ONLY)
      set(US_DOXYGEN_HEADER header.html)

      configure_file(doxygen/footer.html
                     ${CMAKE_CURRENT_BINARY_DIR}/footer.html COPY_ONLY)
      set(US_DOXYGEN_FOOTER footer.html)

      configure_file(doxygen/doxygen.css
                     ${CMAKE_CURRENT_BINARY_DIR}/doxygen.css COPY_ONLY)
      set(US_DOXYGEN_CSS doxygen.css)
      
      set(US_DOXYGEN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/gh-pages CACHE PATH "Doxygen output directory" FORCE)
      set(US_DOXYGEN_HTML_OUTPUT "doc_latest")
    else()
      set(US_DOXYGEN_HEADER )
      set(US_DOXYGEN_FOOTER )
      set(US_DOXYGEN_CSS )
      set(US_DOXYGEN_HTML_OUTPUT "html")
    endif()
    
    # Help doxygen with preprocessor macro expansions
    if(US_NAMESPACE)
      set(US_BEGIN_NAMESPACE "namespace ${US_NAMESPACE} {")
      set(US_END_NAMESPACE "}")
    else()
      set(US_BEGIN_NAMESPACE )
      set(US_END_NAMESPACE )
    endif()
    
    configure_file(doxygen.conf.in
                   ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

    add_custom_target(doc
                      ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      )
  endif()
endif()

