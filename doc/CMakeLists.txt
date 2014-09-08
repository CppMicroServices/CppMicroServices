if(NOT US_IS_EMBEDDED AND NOT US_NO_DOCUMENTATION)
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
    set(US_DOXYGEN_ENABLED_SECTIONS "us_standalone")

    if(US_DOCUMENTATION_FOR_WEBPAGE)
      configure_file(doxygen/header.html
                     ${CMAKE_CURRENT_BINARY_DIR}/header.html COPY_ONLY)
      set(US_DOXYGEN_HEADER header.html)

      configure_file(doxygen/footer.html
                     ${CMAKE_CURRENT_BINARY_DIR}/footer.html COPY_ONLY)
      set(US_DOXYGEN_FOOTER footer.html)

      configure_file(doxygen/doxygen_extra.css
                     ${CMAKE_CURRENT_BINARY_DIR}/doxygen_extra.css COPY_ONLY)
      set(US_DOXYGEN_EXTRA_CSS doxygen_extra.css)

      set(US_DOXYGEN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/gh-pages)
      if(${PROJECT_NAME}_MINOR_VERSION EQUAL 99)
        set(US_DOXYGEN_HTML_OUTPUT "doc_latest")
      else()
        set(US_DOXYGEN_HTML_OUTPUT "doc_${${PROJECT_NAME}_MAJOR_VERSION}_${${PROJECT_NAME}_MINOR_VERSION}")
      endif()
    else()
      set(US_DOXYGEN_HEADER )
      set(US_DOXYGEN_FOOTER )
      set(US_DOXYGEN_CSS )
      set(US_DOXYGEN_HTML_OUTPUT "html")
    endif()

    # Compile a command line tool which transforms comments in CMake scripts into
    # Doxygen parseable C code.
    set(CMakeDoxygenFilter_EXECUTABLE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/CMakeDoxygenFilter${CMAKE_EXECUTABLE_SUFFIX}")
    try_compile(_result_var
                "${CMAKE_CURRENT_BINARY_DIR}"
                "${CMAKE_CURRENT_SOURCE_DIR}/CMakeDoxygenFilter.cpp"
                OUTPUT_VARIABLE _compile_output
                COPY_FILE ${CMakeDoxygenFilter_EXECUTABLE}
               )

    if(NOT _result_var)
      message(FATAL_ERROR "error: Faild to compile ${CMAKE_CURRENT_SOURCE_DIR}/CMakeDoxygenFilter.cpp (result: ${result_var})\n${_compile_output}")
    endif()

    configure_file(doxygen.conf.in
                   ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

    add_custom_target(doc
                      ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                      VERBATIM
                      )

    install(CODE "execute_process(COMMAND \"${CMAKE_COMMAND}\" --build \"${PROJECT_BINARY_DIR}\" --target doc)")

    install(DIRECTORY "${US_DOXYGEN_OUTPUT_DIR}/${US_DOXYGEN_HTML_OUTPUT}"
            DESTINATION "${AUXILIARY_INSTALL_DIR}/doc/"
            COMPONENT doc)
  endif()
endif()
