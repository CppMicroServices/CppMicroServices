Change Log
==========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <http://keepachangelog.com/>`_
and this project adheres to `Semantic Versioning <http://semver.org/>`_.

`Unreleased v3.1.0 <https://github.com/cppmicroservices/cppmicroservices/tree/development>`_ (2017-XX-XX)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.0.0...development>`_

Added
~~~~~

Changed
~~~~~~~

Deprecated
~~~~~~~~~~

- The following methods in BundleContext:
  RemoveBundleListener, RemoveFrameworkListener, RemoveServiceListener.
  The variants of AddBundleListener, AddFrameworkListener, AddServiceListener that take member functions.
- The functions: ServiceListenerMemberFunctor, BundleListenerMemberFunctor and BindFrameworkListenerToFunctor.


Removed
~~~~~~~

Fixed
~~~~~

-  Cannot add more than one listener if its expressed as a lambda.
   `#95 <https://github.com/CppMicroServices/CppMicroServices/issues/95>`_
-  Removing Listeners does not work well
   `#83 <https://github.com/CppMicroServices/CppMicroServices/issues/83>`_

Security
~~~~~~~~

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
