Build Instructions    {#BuildInstructions}
==================

The C++ Micro Services library provides [CMake][cmake] build scripts which allow the generation of
platform and IDE specific project files.

The library should compile on many different platforms. Below is a list of tested compiler/OS combinations:

  - GCC 4.5 (Ubuntu 11.04 and MacOS X 10.6)
  - GCC 4.7 (Ubuntu 12.10)
  - Visual Studio 2008, 2010, and 2012
  - Clang 3.0 (Ubuntu 12.10 and MacOS X 10.6)


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
  Specify if the library should be build shared or static. See \ref MicroServices_StaticModules
  for detailed information about static CppMicroServices modules.
- **US_BUILD_TESTING**
  Build unit tests and code snippets.
- **US_ENABLE_AUTOLOADING_SUPPORT**
  Enable auto-loading of modules located in special sub-directories. See \ref MicroServices_AutoLoading
  for detailed information about this feature.
- **US_ENABLE_THREADING_SUPPORT**
  Enable the use of synchronization primitives (atomics and pthread mutexes or Windows primitives)
  to make the API thread-safe. If your application is not multi-threaded, turn this option OFF
  to get maximum performance.
- **US_ENABLE_RESOURCE_COMPRESSION (advanced)**
  Enable compression of embedded resources. See \ref MicroServices_Resources for detailed information
  about the resource system.

### Customizing naming conventions

- **US_NAMESPACE**
  The default namespace is `us` but you may override this at will.
- **US_HEADER_PREFIX**
  By default, all public headers have a "us" prefix. You may specify an arbitrary prefix to match your
  naming conventions.

The above options are mainly useful when embedding the C++ Micro Services source code in your own library and
you want to make it look like native source code.

[cmake]: http://www.cmake.org
