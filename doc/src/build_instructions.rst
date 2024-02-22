.. _build-instructions:

Build Instructions
==================

The C++ Micro Services library provides `CMake <http://www.cmake.org>`_
build scripts which allow the generation of platform and IDE specific
project or *GNU Make* files.


Prerequisites
-------------

-  `CMake <http://www.cmake.org>`_ 3.17.0 (users of the latest Visual Studio
   should typically also use the latest CMake version available)

Configuration
-------------

When building the C++ Micro Services project, you have a few
configuration options at hand.

 - **CMAKE_INSTALL_PREFIX** The installation path.
 - **US_ENABLE_THREADING_SUPPORT** Enable the use of synchronization
   primitives (atomics and pthread mutexes or Windows primitives) to
   make the API thread-safe. All documented guarantees of thread-safety
   are valid if and only if this option is turned ON. If your
   application is not multi-threaded, turn this option OFF to get
   maximum performance.

   .. note::

      In version 3.0 and 3.1 this option only supported the *ON* value.
      The *OFF* configuration is supported again in version 3.2 and later.

 - **BUILD_SHARED_LIBS** Specify if the library should be build
   shared or static. See :any:`concept-static-bundles`
   for detailed information about static CppMicroServices bundles. 
 - **US_BUILD_TESTING** Build unit tests and code snippets.
 - **US_BUILD_EXAMPLES** Build the tutorial code and other examples.
 - **US_BUILD_DOC_HTML** Build the html documentation, as seen on
   docs.cppmicroservices.org.
 - **US_BUILD_DOC_MAN** Build the man pages. This is typically only
   enabled on a Unix-like system.
 - **US_ENABLE_ASAN** Enable the use of Address Sanitizer in builds of CppMicroServices.

   .. note::

      When this setting is enabled for a CppMicroServices build on Windows, the
      US_ASAN_USER_DLL needs to be correctly set. This DLL is typically found at
      the following location ($VS is the root Visual Studio install folder):
      ``$VS\2019\Community\VC\Tools\MSVC\<version>\bin\Hostx64\x64\clang_rt.asan_dbg_dynamic-x86_64.dll``.

      Additionally, ASAN for Visual Studio is only available in versions 16.9 or later. For
      more information, please see
      the `Microsoft documentation <https://docs.microsoft.com/en-us/cpp/sanitizers/asan?view=msvc-160>`.

 - **US_ASAN_USER_DLL** The path to the ASAN DLL which Microsoft Visual Studio ships.
   This value should only be set on Windows.
 - **US_ENABLE_TSAN** Enable the use of Thread Sanitizer in builds of CppMicroServices.
   This boolean should only be set on Linux or Mac.
 - **US_USE_SYSTEM_GTEST** Build using an existing installation of Google Test.
 - **GTEST_ROOT** Specify the root directory of the Google Test framework
   installation to use when building and running tests.

.. note::

   Building the documentation requires
   
    - a `Python <https://www.python.org>`_ installation,
    - the `Doxygen <http://www.doxygen.org>`_ tool,
    - the `Sphinx <http://www.sphinx-doc.org>`_ documentation generator,
    - and the `breathe <https://github.com/michaeljones/breathe>`_ and
      `Read the Docs Sphinx Theme <https://github.com/snide/sphinx_rtd_theme>`_
      Sphinx extensions
   
   After installing *Python* and *Doxygen*, the remaining dependencies
   can be installed using ``pip``::

      python -m pip install sphinx breathe sphinx_rtd_theme

Building
--------

After configuring a build directory with CMake, the project can be
built. If you chose e.g. *Unix Makefiles*, just type:

.. code:: bash

   make -j

in a terminal with the current directoy set to the build directory.
To install the libraries, header files, and documentation into the
configured **CMAKE_INSTALL_PREFIX** type:

.. code:: bash

   make install

Testing
-------

After building the C++ Micro Services source code, assuming the
**US_BUILD_TESTING** CMake configuration option was set to *ON*,
unit tests can be run. 

To run unit tests, a special Visual Studio and Xcode project named *RUN_TESTS* is created that, when built, will run all tests. For Unix Makefiles, run:

.. code:: bash
   
   make test

Examples of when building and running the C++ Micro Services test suite is useful are:

- Qualifying bug fixes and feature development
- Qualifying the integration of C++ Micro Services with unofficially supported compilers and OSes.

Integration
-----------

Projects using the C++ Micro Services library need to set-up the
correct include paths and link dependencies. Further, each executable
or library which needs a :any:`BundleContext <cppmicroservices::BundleContext>`
must contain specific initialization code, a ``manifest.json`` resource,
and must be compiled with a unique ``US_BUNDLE_NAME`` pre-processor definition.
See :any:`getting-started` for an introduction to the basic concepts.

The C++ Micro Services library provides :any:`cmake-support` for CMake
based projects but there are no restrictions on the type of build system
used for a project.

CMake based projects
~~~~~~~~~~~~~~~~~~~~

To easily set-up include paths and linker dependencies, use the common
``find_package`` mechanism provided by CMake:

.. literalinclude:: /doc/src/CMakeLists.txt
   :language: cmake
   :start-after: [prj-start]
   :end-before: [prj-end]

The CMake code above sets up a basic project (called
*CppMicroServicesExamples*) and tries to find the CppMicroServices package
and subsequently to set the necessary include directories. Building a
shared library might then look like this:

.. literalinclude:: /doc/src/tutorial/dictionaryservice/CMakeLists.txt
   :language: cmake
   :end-before: [doc-end]

The call to :any:`usFunctionGenerateBundleInit` generates the proper
bundle initialization code and provides access to the bundle specific
BundleContext instance. Further, the ``set_property`` command sets the
``US_BUNDLE_NAME`` definition.

Makefile based projects
~~~~~~~~~~~~~~~~~~~~~~~

The following Makefile is located at ``/doc/src/examples/makefile/Makefile``
and demonstrates a minimal build script:

.. literalinclude:: /doc/src/examples/makefile/Makefile
   :language: make

The variable ``CppMicroServices_ROOT`` is an environment variable and
must be set to the CppMicroServices installation directory prior to
invoking ``make``. The bundle initialization code for the
``libbundle.so`` shared library is generated by using the
:any:`CPPMICROSERVICES_INITIALIZE_BUNDLE` pre-processor macro at the end
of the ``bundle.cpp`` source file (any source file compiled into the
bundle would do):

.. literalinclude:: /doc/src/examples/makefile/bundle.cpp
   :language: cpp
   :start-after: [doc-start]
   :end-before: [doc-end]

.. seealso::

   See the :any:`getting-started` section and the general
   :any:`cppmicroservices(7)` documentation to learn more about
   fundamental concepts.
