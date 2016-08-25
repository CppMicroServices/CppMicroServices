Bundle Properties    {#MicroServices_BundleProperties}
=================

A C++ Micro Services Bundle provides meta-data in the form of so-called *properties* about itself.
Properties are key - value pairs where the key is of type `std::string` and the value of type `Any`.
The following properties are always set by the C++ Micro Services library and cannot be altered by
the bundle author:

 * `bundle.id` - The unique id of the bundle (type `long`)
 * `bundle.location` - The full path to the bundle's shared library on the file system (type `std::string`)

Bundle authors must always add the following property to their bundle's `manifest.json` file:

 * `bundle.name` - The human readable name of the bundle (type `std::string`)
 
C++ Micro Services will not install any bundle which doesn't contain a valid 'bundle.name' property in 
its `manifest.json` file.
 
Bundle authors can add custom properties by providing a `manifest.json` file, embedded as a top-level
resource into the bundle (see \ref MicroServices_Resources). The root value of the Json file must be
a Json object. An example `manifest.json` file would be:

~~~{.json}
{
  "bundle.name" : "my bundle",
  "bundle.version" : "1.0.2",
  "bundle.description" : "This bundle provides an awesome service",
  "authors" : [ "John Doe", "Douglas Reynolds", "Daniel Cannady" ],
  "rating" : 5
}
~~~

All Json member names of the root object will be available as property keys in the bundle containing
the `manifest.json` file. The C++ Micro Services library specifies the following standard keys for
re-use in `manifest.json` files:

 * `bundle.version` - The version of the bundle (type `std::string`). The version string must be a
   valid version identifier, as specified in the BundleVersion class.
 * `bundle.vendor` - The vendor name of the bundle (type `std::string`)
 * `bundle.description` - A description for the bundle (type `std::string`)
 * `bundle.autoload_dir` - A custom auto-load directory for the bundle (type `std::string`). If not
   set, this property defaults to the bundle's library name.

\note Some of the properties mentioned above may also be accessed via dedicated methods in the Bundle class,
e.g. Bundle::GetName() or Bundle::GetVersion().

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
