The Bundle Context    {#MicroServices_TheBundleContext}
===================

In the context of the C++ Micro Services library, we will call all supported "shared library" types
(DLL, DSO, DyLib, etc.) uniformly a *bundle*. A bundle accesses the C++ Micro Services API via a
BundleContext object, which is specific to each bundle.

### Creating a BundleContext

To create a BundleContext object for a specific library, you have two options. If your project uses
CMake as the build system, use the supplied `#usFunctionGenerateBundleInit` CMake function to automatically
create a source file and add it to your bundle's sources:

~~~{.cpp}
set(bundle_srcs )
usFunctionGenerateBundleInit(bundle_srcs)
add_library(mylib ${bundle_srcs})
set_property(TARGET ${mylib} APPEND PROPERTY COMPILE_DEFINITIONS US_BUNDLE_NAME=mylib)
~~~

You also need to specify a unique bundle name by using the `US_BUNDLE_NAME` compile definition as
shown in the last line. The bundle name must be a valid C identifier and in the case of
executables is required to be defined to `main`.

If you do not use CMake, you have to add a call to the macro `#US_INITIALIZE_BUNDLE` in one of the source
files of your bundle:

\snippet uServices-bundlecontext/main.cpp InitializeBundle

### Getting a BundleContext

To retrieve the bundle specific BundleContext object from anywhere in your bundle, use the
`#GetBundleContext` function:

\snippet uServices-bundlecontext/main.cpp GetBundleContext

Please note that trying to use `#GetBundleContext` without proper initialization code
in the using shared library while either lead to compile or rumtime errors.
