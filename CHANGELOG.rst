Change Log
==========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <http://keepachangelog.com/>`_
and this project adheres to `Semantic Versioning <http://semver.org/>`_.

`Unreleased v4.0.0 <https://github.com/cppmicroservices/cppmicroservices/tree/development>`_ (2020-XX-XX)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.5.0...development>`_

Added
-----

Changed
-------

Removed
-------

Deprecated
----------

Fixed
-----

v3.5.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.5.0>`_ (2020-07-04)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.4.0...v3.5.0>`_

Added
-----

- `Bundle::GetSymbol API <https://github.com/CppMicroServices/rfcs/blob/master/text/0005-Bundle-Load-API.md>`_
- `SharedLibraryException <https://github.com/CppMicroServices/rfcs/blob/master/text/0004-ds-dlopen-error-handling.md>`_

Changed
-------

- Migrate a handful of tests from the legacy test suite to gtest based test suite
- `Improve shared library loading error messages <https://github.com/CppMicroServices/CppMicroServices/commit/1920dacd4bc11865a66a87b2806a81f0cd6e6e7f>`_
- c++17 compatible
  - https://github.com/CppMicroServices/CppMicroServices/pull/465
  - https://github.com/CppMicroServices/CppMicroServices/pull/479

Removed
-------

- `Remove dead code and partially implemented features <https://github.com/CppMicroServices/CppMicroServices/issues/415>`_
- `Remove code with license conflicts <https://github.com/CppMicroServices/CppMicroServices/issues/419>`_

Deprecated
----------

Fixed
-----

- Correctly install Declarative Services and LogService headers
- `Infinite loop in GetCurrentWorkingDir <https://github.com/CppMicroServices/CppMicroServices/pull/431>`_
- `Use cross build objcopy <https://github.com/CppMicroServices/CppMicroServices/commit/a92460244748b5f12edaaa91ac6bd7ea7ecabdc2>`_
- `Service reference dependency deadlock <https://github.com/CppMicroServices/CppMicroServices/commit/ce0d8bfe505509f0b4cea9ab1b4347532c8b7cbb>`_
- `Instantiating multiple service implementations within the same service component <https://github.com/CppMicroServices/CppMicroServices/commit/48f36a7f06ebce05fd3181c1f32eaf8415cb2a69>`_
- Codecov integration
- `BundleRegistry deadlock <https://github.com/CppMicroServices/CppMicroServices/pull/463>`_
- `Remove unnecessary copying of AnyMap <https://github.com/CppMicroServices/CppMicroServices/pull/468>`_
- `Minimum and maximum cardinality values <https://github.com/CppMicroServices/CppMicroServices/issues/475>`_
- `Error if duplicate service component reference names are used <https://github.com/CppMicroServices/CppMicroServices/pull/474>`_
- `Improve performance of ServiceTrackers <https://github.com/CppMicroServices/CppMicroServices/pull/480>`_


`v3.4.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.4.0>`_ (2019-12-10)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.3.0...v3.4.0>`_

Added
-----
- `Declarative Services <https://github.com/CppMicroServices/rfcs/blob/master/text/0003-declarative-services.md>`_
- `Expose checksum from zip archive. <https://github.com/CppMicroServices/CppMicroServices/issues/307>`_
- Framework property (org.cppmicroservices.library.load.options) to control library loading options on macOS and Linux.
- `Add gmock <https://github.com/CppMicroServices/CppMicroServices/issues/327>`_

Changed
-------

Removed
-------

Deprecated
----------
- The following Bundle method functions:

  - ``GetProperties``
  - ``GetProperty``
  - ``GetPropertyKeys``

Fixed
-----
- `static ServiceTracker object crashes in ServiceTracker::Close() <https://github.com/CppMicroServices/CppMicroServices/issues/281>`_
- `Does the ServiceTracker deleter close the service? <https://github.com/CppMicroServices/CppMicroServices/issues/267>`_
- `Optimize peak heap allocation when installing bundles <https://github.com/CppMicroServices/CppMicroServices/issues/297>`_
- `Change GetHeaders API to return a const ref <https://github.com/CppMicroServices/CppMicroServices/issues/322>`_
- `How do service consumers know whether to use BundleContext::GetService or ServiceObjects? <https://github.com/CppMicroServices/CppMicroServices/issues/325>`_
- `Add a testpoint to validate the return value of ServiceFactory::GetService <https://github.com/CppMicroServices/CppMicroServices/issues/328>`_
- `Invalid Bundle causes crash on method invocation <https://github.com/CppMicroServices/CppMicroServices/issues/263>`_
- `Use correct framework event severity and exception types for service factory errors <https://github.com/CppMicroServices/CppMicroServices/issues/217>`_
- `Raspberry Pi arm build failing <https://github.com/CppMicroServices/CppMicroServices/issues/388>`_
- `Service ctor exception crash <https://github.com/CppMicroServices/CppMicroServices/pull/409>`_
- `Update library loading error messages <https://github.com/CppMicroServices/CppMicroServices/pull/399>`_
- `Unknown Cmake Command "add_compile_definitions" <https://github.com/CppMicroServices/CppMicroServices/issues/412>`_
- `GetChildResources() should not have a dependency on GetChildren() <https://github.com/CppMicroServices/CppMicroServices/issues/397>`_
- Improved code coverage to 90%
- Various performance improvements to:

  * Reduce the number of open file handles
  * Reduce peak heap memory utilization
  * AtCompoundKey
  * ServiceTracker
  * Service look up
  * Bundle installs


`v3.3.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.3.0>`_ (2018-02-20)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.2.0...v3.3.0>`_

Added
-----

- Support constructing long LDAP expressions using concise C++
  `#246 <https://github.com/CppMicroServices/CppMicroServices/issues/246>`_
- Bundle manifest validation
  `#182 <https://github.com/CppMicroServices/CppMicroServices/issues/182>`_

Fixed
-----

- Fix seg faults when using default constructed LDAPFilter
  `#251 <https://github.com/CppMicroServices/CppMicroServices/issues/251>`_

`v3.2.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.2.0>`_ (2017-10-30)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.1.0...v3.2.0>`_

Added
-----

- Code coverage metrics.
  `#219 <https://github.com/CppMicroServices/CppMicroServices/pull/219>`_
- GTest integration.
  `#200 <https://github.com/CppMicroServices/CppMicroServices/issues/200>`_
- Support boolean properties in LDAP filter creation.
  `#224 <https://github.com/CppMicroServices/CppMicroServices/issues/224>`_
- Unicode support.
  `#245 <https://github.com/CppMicroServices/CppMicroServices/pull/245>`_

Changed
-------

- Re-enable single-threaded build configuration.
  `#239 <https://github.com/CppMicroServices/CppMicroServices/pull/239>`_

Fixed
-----

- Fix a race condition when getting and ungetting a service.
  `#202 <https://github.com/CppMicroServices/CppMicroServices/issues/202>`_
- Make reading the current working directory thread-safe.
  `#209 <https://github.com/CppMicroServices/CppMicroServices/issues/209>`_
- Guard against recursive service factory calls.
  `#213 <https://github.com/CppMicroServices/CppMicroServices/issues/213>`_
- Fix LDAP filter match logic to properly handle keys starting with the same sub-string.
  `#227 <https://github.com/CppMicroServices/CppMicroServices/issues/227>`_
- Fix seg fault when using a default constructed LDAPFilter instance.
  `#232 <https://github.com/CppMicroServices/CppMicroServices/issues/232>`_
- Several fixes with respect to error code handling.
  `#238 <https://github.com/CppMicroServices/CppMicroServices/pull/238>`_
- IsConvertibleTo method doesn't check for validity of member.
  `#240 <https://github.com/CppMicroServices/CppMicroServices/issues/240>`_

`v3.1.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.1.0>`_ (2017-06-01)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.0.0...v3.1.0>`_

Changed
~~~~~~~

- Improved BadAnyCastException message. `#181 <https://github.com/CppMicroServices/CppMicroServices/issues/181>`_
- Support installing bundles that do not have .DLL/.so/.dylib file extensions. `#205 <https://github.com/CppMicroServices/CppMicroServices/issues/205>`_

Deprecated
~~~~~~~~~~

- The following BundleContext member functions:

  * ``RemoveBundleListener``
  * ``RemoveFrameworkListener``
  * ``RemoveServiceListener``

  And the variants of

  * ``AddBundleListener``
  * ``AddFrameworkListener``,
  * ``AddServiceListener``

  that take member functions.

- The free functions:

  * ``ServiceListenerMemberFunctor``
  * ``BundleListenerMemberFunctor``
  * ``BindFrameworkListenerToFunctor``

- The functions

  * ``ShrinkableVector::operator[std::size_t]``
  * ``ShrinkableMap::operator[const Key&]``


Fixed
~~~~~

-  Cannot add more than one listener if its expressed as a lambda.
   `#95 <https://github.com/CppMicroServices/CppMicroServices/issues/95>`_
-  Removing Listeners does not work well
   `#83 <https://github.com/CppMicroServices/CppMicroServices/issues/83>`_
-  Crash when trying to acquire bundle context
   `#172 <https://github.com/CppMicroServices/CppMicroServices/issues/172>`_
-  Fix for ``unsafe_any_cast``
   `#198 <https://github.com/CppMicroServices/CppMicroServices/pull/198>`_
-  Stopping a framework while bundle threads are still running may deadlock
   `#210 <https://github.com/CppMicroServices/CppMicroServices/issues/210>`_

`v3.0.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.0.0>`_ (2017-02-08)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v2.1.1...v3.0.0>`_

See the `migration guide <https://github.com/CppMicroServices/CppMicroServices/wiki/Migration-Guide-to-version-3.0>`_
for moving from a 2.x release to 3.x.

Added
~~~~~

-  Added MinGW-w64 to the continuous integration matrix
   `#168 <https://github.com/CppMicroServices/CppMicroServices/pull/168>`_
-  Include major version number in library names and install dirs
   `#144 <https://github.com/CppMicroServices/CppMicroServices/issues/144>`_
-  Integrated coverity scan reports
   `#16 <https://github.com/CppMicroServices/CppMicroServices/issues/16>`_
-  Added OS X to the continuous integration matrix
   `#136 <https://github.com/CppMicroServices/CppMicroServices/pull/136>`_
-  Building for Android is now supported
   `#106 <https://github.com/CppMicroServices/CppMicroServices/issues/106>`_
-  Enhanced the project structure to support sub-projects
   `#14 <https://github.com/CppMicroServices/CppMicroServices/issues/14>`_
-  The bundle life-cycle now supports all states as described by OSGi
   and is controllable by the user
   `#25 <https://github.com/CppMicroServices/CppMicroServices/issues/25>`_
-  Added support for framework listeners and improved logging
   `#40 <https://github.com/CppMicroServices/CppMicroServices/issues/40>`_
-  Implemented framework properties
   `#42 <https://github.com/CppMicroServices/CppMicroServices/issues/42>`_
-  Static bundles embedded into an executable are now auto-installed
   `#109 <https://github.com/CppMicroServices/CppMicroServices/pull/109>`_
-  LDAP queries can now be run against bundle meta-data
   `#53 <https://github.com/CppMicroServices/CppMicroServices/issues/53>`_
-  Resources from bundles can now be accessed without loading their
   shared library
   `#15 <https://github.com/CppMicroServices/CppMicroServices/issues/15>`_
-  Support last modified time for embedded resources
   `#13 <https://github.com/CppMicroServices/CppMicroServices/issues/13>`_

Changed
~~~~~~~

-  Fix up bundle property and manifest header handling
   `#135 <https://github.com/CppMicroServices/CppMicroServices/issues/135>`_
-  Introduced C++11 features
   `#35 <https://github.com/CppMicroServices/CppMicroServices/issues/35>`_
-  Re-organize header files
   `#43 <https://github.com/CppMicroServices/CppMicroServices/issues/43>`_,
   `#67 <https://github.com/CppMicroServices/CppMicroServices/issues/67>`_
-  Improved memory management for framework objects and services
   `#38 <https://github.com/CppMicroServices/CppMicroServices/issues/38>`_
-  Removed static globals
   `#31 <https://github.com/CppMicroServices/CppMicroServices/pull/31>`_
-  Switched to using OSGi nomenclature in class names and functions
   `#46 <https://github.com/CppMicroServices/CppMicroServices/issues/46>`_
-  Improved static bundle support
   `#21 <https://github.com/CppMicroServices/CppMicroServices/issues/21>`_
-  The resource compiler was ported to C++ and gained improved command line options
   `#55 <https://github.com/CppMicroServices/CppMicroServices/issues/55>`_
-  Changed System Bundle ID to ``0``
   `#45 <https://github.com/CppMicroServices/CppMicroServices/issues/45>`_
-  Output exception details (if available) for troubleshooting
   `#27 <https://github.com/CppMicroServices/CppMicroServices/issues/27>`_
-  Using the ``US_DECLARE_SERVICE_INTERFACE`` macro is now optional
   `#24 <https://github.com/CppMicroServices/CppMicroServices/issues/24>`_
-  The ``Any::ToString()`` function now outputs JSON formatted text
   `#12 <https://github.com/CppMicroServices/CppMicroServices/issues/12>`_

Removed
~~~~~~~

-  The autoload feature was removed from the framework
   `#75 <https://github.com/CppMicroServices/CppMicroServices/issues/75>`__

Fixed
~~~~~

-  Headers with ``_p.h`` suffix do not get resolved in Xcode for automatic-tracking of counterparts
   `#93 <https://github.com/CppMicroServices/CppMicroServices/issues/93>`_
-  ``usUtils.cpp`` - Crash can occur if ``FormatMessage(...)`` fails
   `#33 <https://github.com/CppMicroServices/CppMicroServices/issues/33>`_
-  Using ``US_DECLARE_SERVICE_INTERFACE`` with Qt does not work
   `#19 <https://github.com/CppMicroServices/CppMicroServices/issues/19>`_
-  Fixed documentation of public headers.
   `#165 <https://github.com/CppMicroServices/CppMicroServices/issues/165>`_

`v2.1.1 <https://github.com/cppmicroservices/cppmicroservices/tree/v2.1.1>`_ (2014-01-22)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v2.1.0...v2.1.1>`_

Fixed
~~~~~

-  Resource compiler not found error
   `#11 <https://github.com/CppMicroServices/CppMicroServices/issues/11>`_

`v2.1.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v2.1.0>`_ (2014-01-11)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v2.0.0...v2.1.0>`_

Changed
~~~~~~~

-  Use the version number from CMakeLists.txt in the manifest file
   `#10 <https://github.com/CppMicroServices/CppMicroServices/issues/10>`_

Fixed
~~~~~

-  Build fails on Mac OS Mavericks with 10.9 SDK
   `#7 <https://github.com/CppMicroServices/CppMicroServices/issues/7>`_
-  Comparison of service listener objects is buggy on VS 2008
   `#9 <https://github.com/CppMicroServices/CppMicroServices/issues/9>`_
-  Service listener memory leak
   `#8 <https://github.com/CppMicroServices/CppMicroServices/issues/8>`_

`v2.0.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v2.0.0>`_ (2013-12-23)
-----------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v1.0.0...v2.0.0>`_

Major release with backwards incompatible changes. See the `migration guide
<https://github.com/CppMicroServices/CppMicroServices/wiki/API-changes-in-version-2.0.0>`_
for a detailed list of changes.

Added
~~~~~

-  Removed the base class requirement for service objects
-  Improved compile time type checking when working with the service
   registry
-  Added a new service factory class for creating multiple service
   instances based on RFC 195 Service Scopes
-  Added ModuleFindHook and ModuleEventHook classes
-  Added Service Hooks support
-  Added the utility class ``us::LDAPProp`` for creating LDAP filter
   strings fluently
-  Added support for getting file locations for writing persistent data

Removed
~~~~~~~

-  Removed the output stream operator for ``us::Any``

Fixed
~~~~~

-  ``US_ABI_LOCAL`` and symbol visibility for gcc < 4
   `#6 <https://github.com/CppMicroServices/CppMicroServices/issues/6>`_

`v1.0.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v1.0.0>`_ (2013-07-18)
-----------------------------------------------------------------------------------------

Initial release.

Fixed
~~~~~

-  Build fails on Windows with VS 2012 RC due to CreateMutex
   `#5 <https://github.com/CppMicroServices/CppMicroServices/issues/5>`_
-  usConfig.h not added to framework on Mac
   `#4 <https://github.com/CppMicroServices/CppMicroServices/issues/4>`_
-  ``US_DEBUG`` logs even when not in debug mode
   `#3 <https://github.com/CppMicroServices/CppMicroServices/issues/3>`_
-  Segmentation error after unloading module
   `#2 <https://github.com/CppMicroServices/CppMicroServices/issues/2>`_
-  Build fails on Ubuntu 12.04
   `#1 <https://github.com/CppMicroServices/CppMicroServices/issues/1>`_
