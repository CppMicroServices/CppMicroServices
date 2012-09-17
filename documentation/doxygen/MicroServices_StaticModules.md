Static Modules    {#MicroServices_StaticModules}
==============

The normal and most flexible way to include a CppMicroServices module with an application is to compile
it into a shared library that is either linked by another library (or executable) or
\ref MicroServices_AutoLoading "auto-loaded" during runtime.

However, modules can be linked statically to your application or shared library. This makes the deployment
of your application less error-prone and in the case of a complete static build also minimizes its binary
size and start-up time. The disadvantage is that no functionality can be added without a rebuild and
redistribution of the application.

## Creating Static Modules

Static modules are written just like shared modules - there are no differences in the usage of the
CppMicroServices API or the provided preprocessor macros. The only thing you need to make sure is that
the `US_STATIC_MODULE` preprocessor macro is defined when building a module statically.

## Using Static Modules

Static modules can be used (imported) in shared or other static libraries or in the executable itself.
Assuming that a static module makes use of the CppMicroServices API (e.g. by registering some services
using a ModuleContext), the importing library or executable needs to put a call to the `#US_INITIALIZE_MODULE` macro
somewhere in its source code. This ensures the availability of a module context which is shared with all
imported static libraries (see also \ref MicroServices_StaticModules_Context).

\note Note that if your static module does not export a module activator by using the macro
`#US_EXPORT_MODULE_ACTIVATOR` you do not need to put the special import macros explained below into
your code. You can use and link the static module just like any other static library.

For every static module you would like to import, you need to put a call to `#US_IMPORT_MODULE` into the
source code of the importing library. Addidtionally, you need a call to `#US_LOAD_IMPORTED_MODULES`
which contains a space-deliminated list of module names in the importing libaries source code. This ensures
that the module activators of the imported static modules are called appropriately.

\note When importing a static module into another static module, the call to `#US_LOAD_IMPORTED_MODULES` in
the importing static module will have no effect. This macro can only be used in shared modules or executables.

There are two main usage scenarios which are explained below together with some example code.

### Using a Shared CppMicroServices Library

Building the CppMicroServices library as a shared library allows you to import static modules into other
shared or static modules or into the executable. As noted above, the importing shared module or executable
needs to provide a module context by calling the `#US_INITIALIZE_MODULE` macro. Additionally, you must ensure
to use the `#US_LOAD_IMPORTED_MODULES_INTO_MAIN` macro instead of `#US_LOAD_IMPORTED_MODULES` when importing
static modules into an executable.

Example code for importing the two static modules `MyStaticModule1` and `MyStaticModule2` into an executable:

\snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoMain

Importing the static module `MyStaticModule` into a shared or static module looks like this:

\snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoLib

Having a shared CppMicroServices library, the executable also needs some initialization code:

\snippet uServices-staticmodules/main.cpp InitializeExecutable

Note that shared (but not static) modules also need the `#US_INITIALIZE_MODULE` call when importing static modules,
but can omit the US_BUILD_SHARED_LIBS guard.

### Using a Static CppMicroServices Library

The CppMicroServices library can be build as a static library. In that case, creating shared modules is not supported.
If you create shared modules which link a static version of the CppMicroServices library, the runtime behavior is
undefined.

In this usage scenario, every module will be statically build and linked to an executable. The executable needs to
import all the static modules, just like above:

\snippet uServices-staticmodules/main.cpp ImportStaticModuleIntoMain

However, it can omit the `#US_INITIALIZE_MODULE` macro call (the module context from the CppMicroServices library
will be shared across all modules and the executable).

## A Note About The Module Context    {#MicroServices_StaticModules_Context}

Modules using the CppMicroServices API frequently need a `ModuleContext` object to query, retrieve, and register services.
Static modules will never get their own module context but will share the context with their importing module or
executable. Therefore, the importing module or executable needs to ensure the availability of such a context (by using
the `#US_INITIALIZE_MODULE` macro).

\note The CppMicroServices library will *always* provide a module context, independent of its library build mode.

So in a completely statically build application, the CppMicroServices library provides a global module context for all
imported modules and the executable.

