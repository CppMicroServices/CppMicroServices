Build Instructions    {#BuildInstructions}
==================

The C++ Micro Services library provides [CMake][cmake] build scripts which allow the generation of
platform and IDE specific project files.

The library should compile on many different platforms. Below is a list of tested compiler/OS combinations:

  - GCC 4.6 (Ubuntu 12.04)
  - GCC 4.8 (Ubuntu 13.10)
  - Clang 3.2 (Ubuntu 13.10)
  - Clang (MacOS X 10.8, 10.9, 10.10 and 10.11)
  - Visual Studio 2008 SP1, 2010, 2012, 2013, 2015 (Windows 7)


Prerequisites
-------------

- [CMake][cmake] 2.8 (Visual Studio 2010 and 2012 users should use the latest CMake version available)


Configuring the Build
---------------------

When building the C++ Micro Services library, you have a few configuration options at hand.

### General build options

- **CMAKE_INSTALL_PREFIX**
  The installation path.
- **US_BUILD_SHARED_LIBS**
  Specify if the library should be build shared or static. See \ref MicroServices_StaticBundles
  for detailed information about static CppMicroServices bundles.
- **US_BUILD_TESTING**
  Build unit tests and code snippets.
- **US_ENABLE_THREADING_SUPPORT**
  Enable the use of synchronization primitives (atomics and pthread mutexes or Windows primitives)
  to make the API thread-safe. All documented guarantees of thread-safety are valid if and only if
  this option is turned ON. If your application is not multi-threaded, turn this option OFF to get
  maximum performance.

The above options are mainly useful when embedding the C++ Micro Services source code in your own library and
you want to make it look like native source code.

[cmake]: http://www.cmake.org
