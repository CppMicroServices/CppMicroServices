Auto Loading Bundles    {#MicroServices_AutoLoading}
====================

Auto-loading of bundles is a feature of the CppMicroServices library to manage the loading
of bundles which would normally not be loaded at runtime because of missing link-time
dependencies.

The Problem
-----------

Imagine that you have a bundle *A* which provides an interface for loading files and another
bundle *B* which registers a service implementing that interface for files of type *png*.
Your executable *E* uses the interface from *A* to query the service registry for available
services. Due to the link-time dependencies, this results in the following dependency graph:

\dot
digraph linker_deps {
  node [shape=record, fontname=Helvetica, fontsize=10];
  a [ label="Bundle A\n(Interfaces)" ];
  b [ label="Bundle B\n(service provider)" ];
  e [ label="Executable E\n(service consumer)" ];
  a -> e;
  a -> b;
}
\enddot

When the executable *E* is launched, the dynamic linker of your operating system loads
bundle *A* to satisfy the dependencies of *E*, but bundle *B* will not be loaded. Therefore,
the executable will not be able to consume any services from bundle *B*.

The Solution
------------

The problem above is solved in the CppMicroServices library by automatically loading bundles
from a list of configurable file-system locations.

For each bundle being loaded, the following steps are taken:

 - If the bundle provides an activator, it's BundleActivator::Start() method is called.
 - If auto-loading is enabled, and the bundle declared a non-empty auto-load directory, the
   auto-load paths returned from BundleSettings::GetAutoLoadPaths() are processed.
 - For each auto-load path, all bundles in that path with the currently loaded bundle's
   auto-load directory appended are explicitly loaded.

See the BundleSettings class for details about auto-load paths. The auto-load directory of
a bundle defaults to the bundle's library name, but can be customized using a `manifest.json`
file (see \ref MicroServices_BundleProperties). For executables, the auto-load directory
defaults to the special value `main`. This allows third-party bundles to be auto-loaded
during application start-up, without having to reference a special auto-load directory.

If bundle *A* in the example above contains initialization code like

\code
US_INITIALIZE_BUNDLE("Bundle A", "A")
\endcode

and the bundle's library is located at

    /myproject/libA.so

all libraries located at

    /myproject/A/

will be automatically loaded (unless the auto-load paths have been modified). By ensuring that
bundle *B* from the example above is located at

    /myproject/A/libB.so

it will be loaded when the executable *E* is started and is then able to register its services
before the executable queries the service registry.

\note If you need to add additional auto-load search paths during application start-up, provide
a BundleActivator instance in your executable and call BundleSettings::AddAutoLoadPath() in
your executable's BundleActivator::Start() method. If there are bundles inside a `main` sub-directory
of any of the provided auto-load search paths, these bundles will then be auto-loaded before
your executable's main() function is executed.

Environment Variables
---------------------

The following environment variables influence the runtime behavior of the CppMicroServices library:

 - *US_DISABLE_AUTOLOADING* If set, auto-loading of bundles is disabled.
 - *US_AUTOLOAD_PATHS* A `:` (Unix) or `;` (Windows) separated list of paths from which bundles
   should be auto-loaded.
