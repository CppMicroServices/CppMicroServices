[![Build Status](https://secure.travis-ci.org/saschazelzer/CppMicroServices.png)](http://travis-ci.org/saschazelzer/CppMicroServices)

[![Coverity Scan Build Status](https://scan.coverity.com/projects/1329/badge.svg)](https://scan.coverity.com/projects/1329)

C++ Micro Services
==================

Introduction
------------

The C++ Micro Services library provides a dynamic service registry and module system,
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

This is a pure C++ implementation of the OSGi service model and does not have any third-party
library dependencies.

Supported Platforms
-------------------

The library should compile on many different platforms. Below is a list of tested compiler/OS combinations:

  - GCC 4.6 (Ubuntu 12.04)
  - GCC 4.8 (Ubuntu 13.10)
  - Clang 3.2 (Ubuntu 13.10)
  - Clang (MacOS X 10.8 and 10.9)
  - Visual Studio 2008 SP1, 2010, 2012, 2013 (Windows 7)

Legal
-----

Copyright (c) German Cancer Research Center. Licensed under the [Apache License v2.0][apache_license].

Quick Start
-----------

Essentially, the C++ Micro Services library provides you with a powerful dynamic service registry.
Each shared or static library has an associated `ModuleContext` object, through which the service
registry is accessed.

To query the registry for a service object implementing one or more specific interfaces, the code
would look like this:

```cpp
#include <usModuleContext.h>
#include <someInterface.h>

using namespace us;

void UseService(ModuleContext* context)
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
#include <usModuleContext.h>
#include <someInterface.h>

using namespace us;

void RegisterSomeService(ModuleContext* context, SomeInterface* service)
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
