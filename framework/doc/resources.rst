.. _concept-resources:

The Resources System
====================

The C++ Micro Services library provides a generic resources system to
embed arbitrary files into a bundle's shared library (the size
limitation per resource is 2GB, due to the used ZIP format).

The following features are supported:

-  Embed arbitrary data into shared or static bundles or executables.
-  Data is embedded in a compressed format (zip) with a configurable
   compression level.
-  Resources are accessed via a Bundle instance, providing individual
   resource lookup and access for each bundle.
-  Resources are managed in a tree hierarchy, modeling the original
   child - parent relationship on the file-system.
-  The BundleResource class provides a high-level API for accessing
   resource information and traversing the resource tree.
-  The BundleResourceStream class provides an STL input stream derived
   class for the seamless usage of embedded resource data in third-party
   libraries.

The following conventions and limitations apply:

-  Resource entries are stored with case-insensitive names. On
   case-sensitive file systemes, adding resources with the same name but
   different capitalization will lead to an error.
-  Looking up resources by name at runtime *is* case sensitive.
-  The CppMicroServices library will search for a valid zip file inside
   a shared library, starting from the end of the file. If other zip
   files are embedded in the bundle as well (e.g. as an additional
   resource embedded via the Windows RC compiler or using other
   techniques), it will stop at the first valid zip file and use it a
   the resource container.

Embedding Resources in a Bundle
-------------------------------

Resources are embedded into a bundle's shared or static library (or into
an executable) by using the :program:`usResourceCompiler3` executable. It will
create a ZIP archive of all input files and can append it to the bundle
file. See :any:`usResourceCompiler3(1)` for the command line reference.


Accessing Resources at Runtime
------------------------------

Each bundle provides access to its embedded resources via the
:any:`Bundle <cppmicroservices::Bundle>` class which provides methods
returning :any:`BundleResource <cppmicroservices::BundleResource>`
objects. The :any:`BundleResourceStream <cppmicroservices::BundleResourceStream>`
class provides a ``std::istream`` compatible object to access the
resource contents.

The following example shows how to retrieve a resource from each
currently installed bundle whose path is specified by a bundle property:

.. literalinclude:: snippets/uServices-resources/main.cpp
   :language: cpp
   :start-after: [2]
   :end-before: [2]

This example could be enhanced to dynamically react to bundles being
started and stopped, making use of the popular *extender pattern* from
OSGi.

Runtime overhead
----------------

The resources system has the following runtime characteristics:

-  During static initialization of a bundle, it's ZIP archive header
   data (if available) is parsed and stored in memory.
-  Querying ``Bundle`` or ``BundleResource`` objects for resource
   information will not extract the embedded resource data and hence
   only has minimal runtime and memory overhead.
-  Creating a ``BundleResourceStream`` object will allocate memory for
   the uncompressed resource data and inflate it. The memory will be
   free'ed after the ``BundleResourceStream`` object is destroyed.
