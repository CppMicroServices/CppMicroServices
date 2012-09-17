Build Instructions    {#BuildInstructions}
==================

The C++ Micro Services library provides [CMake][cmake] build scripts which allow the generation of
platform and IDE specific project files.

The library should compile on many different platforms. Below is a list of tested compiler/OS combinations:

  - GCC 4.5 (Ubuntu 11.04 and MacOS X 10.6)
  - Visual Studio 2008 and 2010
  - Clang 3.0 (Ubuntu 11.04 and MacOS X 10.6)


Prerequisites
-------------

- [CMake][cmake] 2.8 (Visual Studio 2010 users should use the latest CMake version available)


Configuring the Build
---------------------

When building the C++ Micro Services library, you have a few configuration options at hand.

### General build options

- **US_BUILD_SHARED_LIBS**
  Specify if the library should be build shared or static. See \ref MicroServices_StaticModules for detailed
  information about static CppMicroServices modules.
- **US_BUILD_TESTING**
  Build unit tests and code snippets.
- **US_ENABLE_AUTOLOADING_SUPPORT**
  Enable auto-loading of modules located in special sup-directories. See \ref MicroServices_AutoLoading for
  detailed information about this feature.
- **US_ENABLE_THREADING_SUPPORT**
  Enable the use of synchronization primitives (atomics and pthread mutexes or Windows primitives) to make
  the API thread-safe. If you are application is not multi-threaded, turn this option OFF to get maximum
  performance.
- **US_USE_C++11 (advanced)**
  Enable the usage of C++11 constructs

### Customizing naming conventions

- **US_NAMESPACE**
  The default namespace is `us` but you may override this at will.
- **US_HEADER_PREFIX**
  By default, all public headers have a "us" prefix. You may specify an arbitrary prefix to match your
  naming conventions.

The above options are mainly useful when embedding the C++ Micro Services source code in your own library and
you want to make it look like native source code.

### Configure the service base class

All service implementations must inherit from the same base class. The C++ Micro Services library provides
a trivial class called `us::Base` for that purpose. However, most applications already have a special base
class and you can configure the C++ Micro Services library to use that class.

- **US_BASECLASS_NAME**
  The fully-qualified name of the base class, e.g. `my::OwnBaseClass`. If you don't need support for service
  factories (see below) and don't want to enable US_BUILD_TESTING, this is all you need.
- **US_ENABLE_SERVICE_FACTORY_SUPPORT (advanced)**
  If you want support for service factories (they allow the customization of service objects for individual
  modules, see OSGi Service Platform Core Specification Release 4, Version 4.3, Section 5.6), switch this
  option to ON. If you also provided a custom US_BASECLASS_NAME, you need to set the US_BASECLASS_HEADER variable.
- **US_BASECLASS_HEADER (advanced)**
  The name of the header file containing the declaration for the base class specified in US_BASECLASS_NAME, e.g.
  `myOwnBaseClass.h`. You will also have to set US_BASECLASS_PACKAGE or US_BASECLASS_INCLUDE_DIRS and US_BASECLASS_LIBRARIES.
- **US_BASECLASS_PACKAGE (advanced)**
  If your specified a custom base class from a library which can be found via a CMake `find_package` command, enter
  the package name here. Most of the time, this will correctly set the values for US_BASECLASS_INCLUDE_DIRS,
  US_BASECLASS_LIBRARIES, and US_BASECLASS_LIBRARY_DIRS.
- **US_BASECLASS_INCLUDE_DIRS (advanced)**
  A list of include dirs needed to include the base class header file as specified in US_BASECLASS_HEADER.
- **US_BASECLASS_LIBRARIES (advanced)**
  A list of libraries to link the C++ Micro Services library against for resolving symbols needed for a custom base class.
- **US_BASECLASS_LIBRARY_DIRS (advanced)**
  A list of directories to look for the libraries specified in US_BASECLASS_LIBRARIES

[cmake]: http://www.cmake.org
