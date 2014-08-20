The Module Context    {#MicroServices_TheModuleContext}
===================

In the context of the C++ Micro Services library, we will call all supported "shared library" types
(DLL, DSO, DyLib, etc.) uniformly a *module*. A module accesses the C++ Micro Services API via a
ModuleContext object, which is specific to each module.

### Creating a ModuleContext

To create a ModuleContext object for a specific library, you have two options. If your project uses
CMake as the build system, use the supplied `#usFunctionGenerateModuleInit` CMake function to automatically
create a source file and add it to your module's sources:

~~~{.cpp}
set(module_srcs )
usFunctionGenerateModuleInit(module_srcs)
add_library(mylib ${module_srcs})
set_property(TARGET ${mylib} APPEND PROPERTY COMPILE_DEFINITIONS US_MODULE_NAME=mylib)
~~~

You also need to specify a unique module name by using the `US_MODULE_NAME` compile definition as
shown in the last line. The module name must be a valid C identifier and in the case of
executables is required to be defined to `main`.

If you do not use CMake, you have to add a call to the macro `#US_INITIALIZE_MODULE` in one of the source
files of your module:

\snippet uServices-modulecontext/main.cpp InitializeModule

### Getting a ModuleContext

To retrieve the module specific ModuleContext object from anywhere in your module, use the
`#GetModuleContext` function:

\snippet uServices-modulecontext/main.cpp GetModuleContext

Please note that trying to use `#GetModuleContext` without proper initialization code
in the using shared library while either lead to compile or rumtime errors.
