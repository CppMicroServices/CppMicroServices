Module Properties    {#MicroServices_ModuleProperties}
=================

A C++ Micro Services Module provides meta-data in the form of so-called *properties* about itself.
Properties are key - value pairs where the key is of type `std::string` and the value of type `Any`.
The following properties are always set by the C++ Micro Services library and cannot be altered by
the module author:

 * `module.id` - The unique id of the module (type `long`)
 * `module.name` - The human readable name of the module (type `std::string`)
 * `module.location` - The full path to the module's shared library on the file system (type `std::string`)

Module authors can add custom properties by providing a `manifest.json` file, embedded as a top-level
resource into the module (see \ref MicroServices_Resources). The root value of the Json file must be
a Json object. An example `manifest.json` file would be:

~~~{.json}
{
  "module.version" : "1.0.2",
  "module.description" : "This module provides an awesome service",
  "authors" : [ "John Doe", "Douglas Reynolds", "Daniel Cannady" ],
  "rating" : 5
}
~~~

All Json member names of the root object will be available as property keys in the module containing
the `manifest.json` file. The C++ Micro Services library specifies the following standard keys for
re-use in `manifest.json` files:

 * `module.version` - The version of the module (type `std::string`). The version string must be a
   valid version identifier, as specified in the ModuleVersion class.
 * `module.vendor` - The vendor name of the module (type `std::string`)
 * `module.description` - A description for the module (type `std::string`)
 * `module.autoload_dir` - A custom auto-load directory for the module (type `std::string`). If not
   set, this property defaults to the module's library name.

\note Some of the properties mentioned above may also be accessed via dedicated methods in the Module class,
e.g. Module::GetName() or Module::GetVersion().

When parsing the `manifest.json` file, the Json types are mapped to C++ types and stored in instances of
the Any class. The mapping is as follows:

| Json  | C++ (%Any) |
|-------|-----------|
|object | std::map<std::string,%Any>        |
|array  | std::vector<Any> |
|string | std::string |
|number | int or double |
|true   | bool |
|false  | bool |
|null   | %Any() |
