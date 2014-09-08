Static Modules    {#MicroServices_StaticModules}
==============

The normal and most flexible way to include a CppMicroServices module in an application is to compile
it into a shared library that is either linked by another library (or executable) or
\ref MicroServices_AutoLoading "auto-loaded" during runtime.

However, modules can be linked statically to your application or shared library. This makes the deployment
of your application less error-prone and in the case of a complete static build also minimizes its binary
size and start-up time. The disadvantage is that no functionality can be added without a rebuild and
redistribution of the application.

# Creating Static Modules

Static modules are written just like shared modules - there are no differences in the usage of the
CppMicroServices API or the provided preprocessor macros. The only thing you need to make sure is that
the `US_STATIC_MODULE` preprocessor macro is defined when building a module statically.

# Using Static Modules

Static modules can be used (imported) in shared or other static libraries or in the executable itself.
For every static module you would like to import, you need to put a call to `#US_IMPORT_MODULE` or
to `#US_INITIALIZE_STATIC_MODULE` (if the module does not provide an activator) into the
source code of the importing library.

\note While you can link static modules to other static modules, you will still need to
import *all* of the static modules into the final executable to ensure proper initialization.

There are two main usage scenarios which are explained below together with some example code.

## Using a Shared CppMicroServices Library

Building the CppMicroServices library as a shared library allows you to import static modules into other
shared or static modules or into the executable.

Example code for importing the static module `MyStaticModule1` into another library
or executable:

\snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoMain

## Using a Static CppMicroServices Library

The CppMicroServices library can be build as a static library. In that case, creating shared
modules is not supported. If you create shared modules which link a static version of the
CppMicroServices library, the runtime behavior is undefined.

In this usage scenario, every module will be statically build and linked to an executable:

\snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoMain2

Note that the first `#US_IMPORT_MODULE` call imports the static CppMicroServices library.
Then the `MyStaticModule2` module is imported and finally, the
executable itself is initialized (this is necessary if the executable itself is
a C++ Micro Services module).

# A Note About Import Ordering    {#MicroServices_StaticModules_Order}

Although static linking reduces the number of shared libraries, the statically
linked modules are still represented internally by distinct Module and ModuleContext
objects. In a shared module scenario, the linker dependencies define the load order
of modules and hence the order of their initialization and ModuleActivator::Load()
invocations. In the static case, the order of the `#US_IMPORT_MODULE` macro calls
defines the initialization and activation order. In case of a statically built
CppMicroServices library, it is therefore important to always import the static
CppMicroServices library into the executable first, and the executable itself
(if it needs to be initialized) last.
