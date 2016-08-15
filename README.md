Branch | Linux | Windows
-------|-------|---------
master | [![Linux Build Status](https://secure.travis-ci.org/CppMicroServices/CppMicroServices.png)](http://travis-ci.org/CppMicroServices/CppMicroServices) | [![Windows Build status](https://ci.appveyor.com/api/projects/status/ieoylxs37tjui4sc?svg=true)](https://ci.appveyor.com/project/saschazelzer/cppmicroservices)
development | [![Linux Build Status (development)](https://travis-ci.org/CppMicroServices/CppMicroServices.svg?branch=development)](https://travis-ci.org/CppMicroServices/CppMicroServices) | [![Windows Build status (development)](https://ci.appveyor.com/api/projects/status/ieoylxs37tjui4sc/branch/development?svg=true)](https://ci.appveyor.com/project/saschazelzer/cppmicroservices/branch/development)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/1329/badge.svg)](https://scan.coverity.com/projects/1329)

C++ Micro Services
==================

Introduction
------------

The C++ Micro Services library provides a dynamic service registry and bundle system,
partially based the OSGi Core Release 5 specifications. It enables developers to create
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

  - GCC 4.6.3 (Ubuntu 14.10)
  - GCC 4.9.2 (Ubuntu 15.04)
  - GCC 5.1.1 (Fedora 22)
  - Clang 3.6.0 (Ubuntu 15.04)
  - Visual Studio 2013 Update 5 (Windows 8.1)
  - Visual Studio 2015 (Windows 8.1)

Note: MacOS was supported in the past but cannot be tested currently due to the lack
of Apple hardware and software.

Legal
-----

The C++ Micro Services project was initially developed at the German Cancer Research Center.
Its source code is hosted on the GitHub account of the primary author, located at
https://github.com/CppMicroServices/CppMicroServices. See the COPYRIGHT file in the top-level
directory for detailed copyright information.

This project is licensed under the [Apache License v2.0][apache_license].

Quick Start
-----------

Essentially, the C++ Micro Services library provides you with a powerful dynamic service registry.
Each shared or static library has an associated `BundleContext` object, through which the service
registry is accessed.

To query the registry for a service object implementing one or more specific interfaces, the code
would look like this:

```cpp
#include "cppmicroservices/BundleContext.h"
#include "someInterface.h"

using namespace cppmicroservices;

void UseService(BundleContext* context)
{
  ServiceReference serviceRef = context->GetServiceReference<SomeInterface>();
  if (serviceRef)
  {
    SomeInterface* service = context->GetService<SomeInterface>(serviceRef);
    if (service) { /* do something */ }
  }
}
```

Registering a service object against a certain interface looks like this:

```cpp
#include "cppmicroservices/BundleContext.h"
#include "someInterface.h"

using namespace cppmicroservices;

void RegisterSomeService(BundleContext* context, SomeInterface* service)
{
  context->RegisterService<SomeInterface>(service);
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
