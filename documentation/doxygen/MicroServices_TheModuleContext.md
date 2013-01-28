The Module Context    {#MicroServices_TheModuleContext}
===================

In the context of the C++ Micro Services library, we will call all supported "shared library" types
(DLL, DSO, DyLib, etc.) uniformly a *module*. A module accesses the C++ Micro Services API via a
ModuleContext object. While multiple modules could use the same ModuleContext, it is highly recommended
that each module gets its own (this will enable module specific service usage tracking and also allows
the C++ Micro Services framework to properly cleanup resources after a module has been unloaded).

### Creating a ModuleContext

To create a ModuleContext object for a specific library, you have two options. If your project uses
CMake as the build system, use the supplied `usFunctionGenerateModuleInit` CMake function to automatically
create a source file and add it to your module's sources:

    set(module_srcs )
    usFunctionGenerateModuleInit(module_srcs
                                 NAME "My Module"
                                 LIBRARY_NAME "mylibname"
                                 VERSION "1.0.0"
                                )
    add_library(mylib ${module_srcs})
    
If you do not use CMake, you have to add a call to the macro `#US_INITIALIZE_MODULE` in one of the source
files of your module:

\snippet uServices-modulecontext/main.cpp InitializeModule

### Getting a ModuleContext

To retrieve the module specific ModuleContext object from anywhere in your module, use the
`#GetModuleContext` function:

\snippet uServices-modulecontext/main.cpp GetModuleContext

Please note that the call to `#GetModuleContext` will fail if you did not create a module specific context.
