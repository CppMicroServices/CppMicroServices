Static Bundles    {#MicroServices_StaticBundles}
==============

The normal and most flexible way to include a CppMicroServices bundle in an application is to compile
it into a shared library that is linked by another shared library (or executable).

However, bundles can be linked statically to your application or shared library. This makes the deployment
of your application less error-prone and in the case of a complete static build also minimizes its binary
size and start-up time. The disadvantage is that no functionality can be added without a rebuild and
redistribution of the application.

# Creating Static Bundles

Static bundles are written just like shared bundles - there are no differences in the usage of the
CppMicroServices API or the provided preprocessor macros.

# Using Static Bundles

Static bundles can be used (imported) in shared or other static libraries or in the executable itself.
For every static bundle you would like to import, you need to put a call to `#CPPMICROSERVICES_IMPORT_BUNDLE` or
to `#CPPMICROSERVICES_INITIALIZE_STATIC_BUNDLE` (if the bundle does not provide an activator) into the
source code of the importing library.

\note While you can link static bundles to other static bundles, you will still need to
import *all* of the static bundles into the final executable to ensure proper initialization.

There are two main usage scenarios which are explained below together with some example code.

## Using a Shared CppMicroServices Library

Building the CppMicroServices library as a shared library allows you to import static bundles into other
shared or static bundles or into the executable.

Example code for importing the static bundle `MyStaticBundle1` into another library
or executable:

\snippet uServices-staticbundles/main.cpp ImportStaticBundleIntoMain

## Using a Static CppMicroServices Library

The CppMicroServices library can be build as a static library. In that case, creating shared
bundles is not supported. If you create shared bundles which link a static version of the
CppMicroServices library, the runtime behavior is undefined.

In this usage scenario, every bundle will be statically build and linked to an executable:

\snippet uServices-staticbundles/main.cpp ImportStaticBundleIntoMain2

Note that the first `#CPPMICROSERVICES_IMPORT_BUNDLE` call imports the static CppMicroServices library.
Then the `MyStaticBundle2` bundle is imported and finally, the
executable itself is initialized (this is necessary if the executable itself is
a C++ Micro Services bundle).

# A Note About Import Ordering    {#MicroServices_StaticBundles_Order}

Although static linking reduces the number of shared libraries, the statically
linked bundles are still represented internally by distinct Bundle and BundleContext
objects. In a shared bundle scenario, the linker dependencies define the load order
of bundles and hence the order of their initialization and BundleActivator::Start()
invocations. In the static case, the order of the `#CPPMICROSERVICES_IMPORT_BUNDLE` macro calls
defines the initialization and activation order. In case of a statically built
CppMicroServices library, it is therefore important to always import the static
CppMicroServices library into the executable first, and the executable itself
(if it needs to be initialized) last.
