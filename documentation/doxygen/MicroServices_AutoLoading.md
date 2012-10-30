Auto Loading Modules    {#MicroServices_AutoLoading}
====================

Auto-loading of modules is a feature of the CppMicroServices library to manage the loading
of modules which would normally not be loaded at runtime because of missing link-time
dependencies.

The Problem
-----------

Imagine that you have a module *A* which provides an interface for loading files and another
module *B* which registers a service implementing that interface for files of type *png*.
Your executable *E* uses the interface from *A* to query the service registry for available
services. Due to the link-time dependencies, this results in the following dependency graph:

\dot
digraph linker_deps {
  node [shape=record, fontname=Helvetica, fontsize=10];
  a [ label="Module A\n(Interfaces)" ];
  b [ label="Module B\n(service provider)" ];
  e [ label="Executable E\n(service consumer)" ];
  a -> e;
  a -> b;
}
\enddot

When the executable *E* is launched, the dynamic linker of your operating system loads
module *A* to satisfy the dependencies of *E*, but module *B* will not be loaded. Therefore,
the executable will not be able to consume any services from module *B*.

The Solution
------------

The problem above is solved in the CppMicroServices library by automatically loading modules
from a list of configurable file-system locations.

For each module being loaded, the following steps are taken:

 - If the module provides an activator, it's ModuleActivator::Load() method is called.
 - If auto-loading is enabled, and the module declared a non-empty auto-load directory, the
   auto-load paths returned from ModuleSettings::GetAutoLoadPaths() are processed.
 - For each auto-load path, all modules in that path with the currently loaded module's
   auto-load directory appended are explicitly loaded.

See the ModuleSettings class for details about auto-load paths and the #US_INITIALIZE_MODULE
macro for details about a module's auto-load directory.

If module *A* in the example above contains initialization code like

\code
US_INITIALIZE_MODULE("Module A", "A", "", "1.0.0")
\endcode

and the module's library is located at

    /myproject/libA.so

all libraries located at

    /myproject/A/

will be automatically loaded (unless the auto-load paths have been modified). By ensuring that
module *B* from the example above is located at

    /myproject/A/libB.so

it will be loaded when the executable *E* is started and is then able to register its services
before the executable queries the service registry.

Environment Variables
---------------------

The following environment variables influence the runtime behavior of the CppMicroServices library:
 
 - *US_DISABLE_AUTOLOADING* If set, auto-loading of modules is disabled.
 - *US_AUTOLOAD_PATHS* A `:` (Unix) or `;` (Windows) separated list of paths from which modules
   should be auto-loaded.

