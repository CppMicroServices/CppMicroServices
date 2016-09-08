The Resources System    {#MicroServices_Resources}
====================

The C++ Micro Services library provides a generic resources system to embed arbitrary files into a
bundle's shared library (the size limitation per resource is 2GB, due to the used ZIP format).

The following features are supported:

 * Embed arbitrary data into shared or static bundles or executables.
 * Data is embedded in a compressed format (zip) with a configurable compression level.
 * Resources are accessed via a Bundle instance, providing individual resource lookup and access
   for each bundle.
 * Resources are managed in a tree hierarchy, modeling the original child - parent relationship
   on the file-system.
 * The BundleResource class provides a high-level API for accessing resource information and
   traversing the resource tree.
 * The BundleResourceStream class provides an STL input stream derived class for the seamless usage
   of embedded resource data in third-party libraries.

The following conventions and limitations apply:

 * Resource entries are stored with case-insensitive names. On case-sensitive file systemes,
   adding resources with the same name but different capitalization will lead to an error.
 * Looking up resources by name at runtime *is* case sensitive.
 * The CppMicroServices library will search for a valid zip file inside a shared library,
   starting from the end of the file. If other zip files are embedded in the bundle as
   well (e.g. as an additional resource embedded via the Windows RC compiler or using
   other techniques), it will stop at the first valid zip file and use it a the resource
   container.

Embedding Resources in a %Bundle
--------------------------------

Resources are embedded into a bundle's shared or static library (or into an executable)
by using the `usResourceCompiler` executable. It will create a ZIP archive of all input
files and can append it to the bundle file.

If you are using CMake, consider using the provided `#usFunctionEmbedResources` CMake macro which
handles the invocation of the `usResourceCompiler` executable and sets up the correct file
dependencies. Otherwise, you also need to make sure that the set of static bundles linked
into a shared bundle or executable is also in the input file list of your `usResourceCompiler`
call for that shared bundle or executable.

Here is a full example creating a bundle and embedding resource data:
\include uServices-resources-cmake/CMakeLists_example.txt

If you are not using CMake, you can run the resource compiler from the terminal.

Example usage of resource compiler:    
1. To create a zip file with resources    
    <code>usResourceCompiler --bundle-name mybundle --out-file Example.zip --manifest-add manifest.json --res-add icon.png</code>    
2. To include a resource file into a bundle    
    <code>usResourceCompiler --bundle-name mybundle --bundle-file mybundle.so --manifest-add manifest.json</code>    
3. To include the contents of zip file into a bundle    
    <code>usResourceCompiler --bundle-file mybundle.so --zip-add archivetomerge.zip</code>

run <code>usResourceCompiler --help</code> for full list of options and usage

Accessing Resources at Runtime
------------------------------

Each bundle provides access to its embedded resources via the Bundle class which provides methods
returning BundleResource objects. The BundleResourceStream class provides a std::istream compatible
object to access the resource contents.

The following example shows how to retrieve a resource from each currently installed bundle whose path
is specified by a bundle property:

\snippet uServices-resources/main.cpp 2

This example could be enhanced to dynamically react to bundles being started and stopped, making use
of the popular "extender pattern" from OSGi.

Runtime overhead
----------------

The resources system has the following runtime characteristics:

 * During static initialization of a bundle, it's ZIP archive header data (if available)
   is parsed and stored in memory.
 * Querying `Bundle` or `BundleResource` objects for resource information will not
   extract the embedded resource data and hence only has minimal runtime and memory
   overhead.
 * Creating a `BundleResourceStream` object will allocate memory for the uncompressed
   resource data and inflate it. The memory will be free'ed after the `BundleResourceStream`
   object is destroyed.
