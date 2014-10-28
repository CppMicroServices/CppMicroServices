The Resources System    {#MicroServices_Resources}
====================

The C++ Micro Services library provides a generic resources system to embed arbitrary files into a
module's shared library (the size limitation per resource is 2GB, due to the used ZIP format).

The following features are supported:

 * Embed arbitrary data into shared or static modules or executables.
 * Data is embedded in a compressed format (zip) with a configurable compression level.
 * Resources are accessed via a Module instance, providing individual resource lookup and access
   for each module.
 * Resources are managed in a tree hierarchy, modeling the original child - parent relationship
   on the file-system.
 * The ModuleResource class provides a high-level API for accessing resource information and
   traversing the resource tree.
 * The ModuleResourceStream class provides an STL input stream derived class for the seamless usage
   of embedded resource data in third-party libraries.

The following conventions and limitations apply:

 * Resource entries are stored with case-insensitive names. On case-sensitive file systemes,
   adding resources with the same name but different capitalization will lead to an error.
 * Looking up resources by name at runtime *is* case sensitive.
 * The CppMicroServices library will search for a valid zip file inside a shared library,
   starting from the end of the file. If other zip files are embedded in the module as
   well (e.g. as an additional resource embedded via the Windows RC compiler or using
   other techniques), it will stop at the first valid zip file and use it a the resource
   container.

Embedding Resources in a %Module
--------------------------------

Resources are embedded into a module's shared or static library (or into an executable)
by using the `usResourceCompiler` executable. It will create a ZIP archive of all input
files and can append it to the module file.

If you are using CMake, consider using the provided `#usFunctionEmbedResources` CMake macro which
handles the invocation of the `usResourceCompiler` executable and sets up the correct file
dependencies. Otherwise, you also need to make sure that the set of static modules linked
into a shared module or executable is also in the input file list of your `usResourceCompiler`
call for that shared module or executable.

Here is a full example creating a module and embedding resource data:
\include uServices-resources-cmake/CMakeLists_example.txt

Accessing Resources at Runtime
------------------------------

Each module provides access to its embedded resources via the Module class which provides methods
returning ModuleResource objects. The ModuleResourceStream class provides a std::istream compatible
object to access the resource contents.

The following example shows how to retrieve a resource from each currently loaded module whose path
is specified by a module property:

\snippet uServices-resources/main.cpp 2

This example could be enhanced to dynamically react to modules being loaded and unloaded, making use
of the popular "extender pattern" from OSGi.

Runtime overhead
----------------

The resources system has the following runtime characteristics:

 * During static initialization of a module, it's ZIP archive header data (if available)
   is parsed and stored in memory.
 * Querying `Module` or `ModuleResource` objects for resource information will not
   extract the embedded resource data and hence only has minimal runtime and memory
   overhead.
 * Creating a `ModuleResourceStream` object will allocate memory for the uncompressed
   resource data and inflate it. The memory will be free'ed after the `ModuleResourceStream`
   object is destroyed.

