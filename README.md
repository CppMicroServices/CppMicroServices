Branch | Linux (Ubuntu 12.04) | Windows (VS 2013 & 2015)
-------|----------------------|-------------------------
master | [![Linux Build Status](https://img.shields.io/travis/CppMicroServices/CppMicroServices/master.svg?style=flat-square&label=Linux)](http://travis-ci.org/CppMicroServices/CppMicroServices) | [![Windows Build status](https://img.shields.io/appveyor/ci/cppmicroservices/cppmicroservices/master.svg?style=flat-square&label=Windows)](https://ci.appveyor.com/project/cppmicroservices/cppmicroservices/branch/master)
development | [![Linux Build Status (development)](https://img.shields.io/travis/CppMicroServices/CppMicroServices/development.svg?style=flat-square&label=Linux)](https://travis-ci.org/CppMicroService/CppMicroServices) | [![Windows Build status (development)](https://img.shields.io/appveyor/ci/cppmicroservices/cppmicroservices/development.svg?style=flat-square&label=Windows)](https://ci.appveyor.com/project/cppmicroservices/cppmicroservices/branch/development)

[![Coverity Scan Build Status](https://img.shields.io/coverity/scan/1329.svg?style=flat-square)](https://scan.coverity.com/projects/1329)

C++ Micro Services
==================

Introduction
------------

The C++ Micro Services library provides a dynamic service registry and bundle system,
largely based the OSGi Core Release 5 specifications. It enables developers to create
a service oriented and dynamic software stack.

Proper usage of the C++ Micro Services library leads to

  - Re-use of software components
  - Loose coupling
  - Separation of concerns
  - Clean APIs based on service interfaces
  - Extensible systems

and more.

Requirements
------------

This is a pure C++ implementation of the OSGi service model and a native version of
OSGi bundles. It does not have any third-party library dependencies.

Supported Platforms
-------------------

The library makes use of some C++11 features and compiles on many different platforms.

Minimum required compiler versions:

  - GCC 4.6
  - Clang 3.1
  - Visual Studio 2013

Below is a list of tested compiler/OS combinations:

  - GCC 4.6.3 (Ubuntu 12.04) via TravisCI
  - GCC 6.1.1 (Fedora 24)
  - Clang 3.8.0 (Fedora 24)
  - Visual Studio 2013 via Appveyor
  - Visual Studio 2015 via Appveyor
  - OS X 10.10

Legal
-----

The C++ Micro Services project was initially developed at the German Cancer Research Center.
Its source code is hosted on the GitHub account of the primary author, located at
https://github.com/CppMicroServices/CppMicroServices. See the COPYRIGHT file in the top-level
directory for detailed copyright information.

This project is licensed under the [Apache License v2.0][apache_license].

Quick Start
-----------

Essentially, the C++ Micro Services library provides you with a powerful dynamic service registry,
on top of a managed lifecycle for so-called *bundles* that are contained in shared or static libraries.
Each bundle within a library has an associated `BundleContext` object, through which the service
registry is accessed.

To query the registry for a service object implementing one or more specific interfaces, the code
would look like this:

```cpp
#include "cppmicroservices/BundleContext.h"
#include "SomeInterface.h"

using namespace cppmicroservices;

void UseService(BundleContext context)
{
  auto serviceRef = context.GetServiceReference<SomeInterface>();
  if (serviceRef)
  {
    auto service = context.GetService(serviceRef);
    if (service) { /* do something */ }
  }
}
```

Registering a service object against a certain interface looks like this:

```cpp
#include "cppmicroservices/BundleContext.h"
#include "SomeInterface.h"

using namespace cppmicroservices;

void RegisterSomeService(BundleContext context, const std::shared_ptr<SomeInterface>& service)
{
  context.RegisterService<SomeInterface>(service);
}
```

The OSGi service model additionally allows to annotate services with properties and using these
properties during service look-ups. It also allows to track the life-cycle of service objects.
Please see the [Documentation](http://cppmicroservices.org/doc_latest/index.html) for more
examples and tutorials and the API reference. There is also a blog post about
[OSGi Lite for C++](http://blog.cppmicroservices.org/2012/04/15/osgi-lite-for-c++).

Build Instructions
------------------

Please visit the [Build Instructions][bi_master] page online.

[bi_master]: http://cppmicroservices.org/doc_latest/BuildInstructions.html
[apache_license]: http://www.apache.org/licenses/LICENSE-2.0
