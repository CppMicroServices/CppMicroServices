[![Build Status](https://secure.travis-ci.org/saschazelzer/CppMicroServices.png)](http://travis-ci.org/saschazelzer/CppMicroServices)

C++ Micro Services
==================

Introduction
------------

The C++ Micro Services library provides a dynamic service registry based on the
service layer as specified in the OSGi R4.2 specifications. It enables users to
realize a service oriented approach within their software stack.

The advantages include higher reuse of components, looser coupling, better organization of
responsibilities, cleaner API contracts, etc.

Requirements
------------

This is a pure C++ implementation of the OSGi service model and does not have any third-party
library dependencies.

Supported Platforms
-------------------

The library should compile on many different platforms. Below is a list of tested compiler/OS combinations:

  - GCC 4.5 (Ubuntu 11.04 and MacOS X 10.6)
  - Visual Studio 2008 and 2010
  - Clang 3.0 (Ubuntu 11.04 and MacOS X 10.6)
  
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
