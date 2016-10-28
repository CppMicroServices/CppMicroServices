# Change Log

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) 
and this project adheres to [Semantic Versioning](http://semver.org/).


## [Unreleased 3.x](https://github.com/cppmicroservices/cppmicroservices/tree/development)

### Added
### Changed
### Deprecated
### Removed
### Fixed
### Security


## [Unreleased 3.0](https://github.com/cppmicroservices/cppmicroservices/tree/development)

[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v2.1.1...development)

See the [migration guide](https://github.com/CppMicroServices/CppMicroServices/wiki/Migration-Guide-to-version-3.0)
for moving from a 2.x release to 3.x.

### Added

- Integrated coverity scan reports [\#16](https://github.com/CppMicroServices/CppMicroServices/issues/16)
- Added OS X to the continuous integration matrix [\#136](https://github.com/CppMicroServices/CppMicroServices/pull/136)
- Building for Android is now supported [\#106](https://github.com/CppMicroServices/CppMicroServices/issues/106)
- Enhanced the project structure to support sub-projects [\#14](https://github.com/CppMicroServices/CppMicroServices/issues/14)
- The bundle life-cycle now supports all states as described by OSGi and is controllable by the user [\#25](https://github.com/CppMicroServices/CppMicroServices/issues/25)
- Added support for framework listeners and improved logging [\#40](https://github.com/CppMicroServices/CppMicroServices/issues/40)
- Implemented framework properties [\#42](https://github.com/CppMicroServices/CppMicroServices/issues/42)
- Static bundles embedded into an executable are now auto-installed [\#109](https://github.com/CppMicroServices/CppMicroServices/pull/109)
- LDAP queries can now be run against bundle meta-data [\#53](https://github.com/CppMicroServices/CppMicroServices/issues/53)
- Resources from bundles can now be accessed without loading their shared library [\#15](https://github.com/CppMicroServices/CppMicroServices/issues/15)
- Support last modified time for embedded resources [\#13](https://github.com/CppMicroServices/CppMicroServices/issues/13)

### Changed

- Introduced C++11 features [\#35](https://github.com/CppMicroServices/CppMicroServices/issues/35)
- Re-organize header files [\#43](https://github.com/CppMicroServices/CppMicroServices/issues/43), [\#67](https://github.com/CppMicroServices/CppMicroServices/issues/67)
- Improved memory management for framework objects and services [\#38](https://github.com/CppMicroServices/CppMicroServices/issues/38)
- Removed static globals [\#31](https://github.com/CppMicroServices/CppMicroServices/pull/31)
- Switched to using OSGi nomenclature in class names and functions [\#46](https://github.com/CppMicroServices/CppMicroServices/issues/46)
- Improved static bundle support [\#21](https://github.com/CppMicroServices/CppMicroServices/issues/21)
- The resource compiler was ported to C++ and gained improved command line options [\#55](https://github.com/CppMicroServices/CppMicroServices/issues/55)
- Changed System Bundle ID to `0` [\#45](https://github.com/CppMicroServices/CppMicroServices/issues/45)
- Output exception details \(if available\) for troubleshooting [\#27](https://github.com/CppMicroServices/CppMicroServices/issues/27)
- Using the `US_DECLARE_SERVICE_INTERFACE` macro is now optional [\#24](https://github.com/CppMicroServices/CppMicroServices/issues/24)
- The `Any::ToString()` function now outputs JSON formatted text [\#12](https://github.com/CppMicroServices/CppMicroServices/issues/12)

### Removed

- The autoload feature was removed from the framework [\#75](https://github.com/CppMicroServices/CppMicroServices/issues/75)

### Fixed

- Headers with `_p.h` suffix do not get resolved in Xcode for automatic-tracking of counterparts [\#93](https://github.com/CppMicroServices/CppMicroServices/issues/93)
- `usUtils.cpp` - Crash can occur if FormatMessage\(...\) fails [\#33](https://github.com/CppMicroServices/CppMicroServices/issues/33)
- Using `US_DECLARE_SERVICE_INTERFACE` with Qt does not work [\#19](https://github.com/CppMicroServices/CppMicroServices/issues/19)


## [v2.1.1](https://github.com/cppmicroservices/cppmicroservices/tree/v2.1.1) (2014-01-22)

[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v2.1.0...v2.1.1)

### Fixed

- Resource compiler not found error [\#11](https://github.com/CppMicroServices/CppMicroServices/issues/11)

## [v2.1.0](https://github.com/cppmicroservices/cppmicroservices/tree/v2.1.0) (2014-01-11)

[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v2.0.0...v2.1.0)

### Changed

- Use the version number from CMakeLists.txt in the manifest file [\#10](https://github.com/CppMicroServices/CppMicroServices/issues/10)

### Fixed

- Build fails on Mac OS Mavericks with 10.9 SDK [\#7](https://github.com/CppMicroServices/CppMicroServices/issues/7)
- Comparison of service listener objects is buggy on VS 2008 [\#9](https://github.com/CppMicroServices/CppMicroServices/issues/9)
- Service listener memory leak [\#8](https://github.com/CppMicroServices/CppMicroServices/issues/8)

## [v2.0.0](https://github.com/cppmicroservices/cppmicroservices/tree/v2.0.0) (2013-12-23)

[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v1.0.0...v2.0.0)

Major release with backwards incompatible changes. See the [migration guide](https://github.com/CppMicroServices/CppMicroServices/wiki/API-changes-in-version-2.0.0)
for a detailed list of changes.

### Added

- Removed the base class requirement for service objects
- Improved compile time type checking when working with the service registry
- Added a new service factory class for creating multiple service instances based on RFC 195 Service Scopes
- Added ModuleFindHook and ModuleEventHook classes
- Added Service Hooks support
- Added the utility class `us::LDAPProp` for creating LDAP filter strings fluently
- Added support for getting file locations for writing persistent data

### Removed

- Removed the output stream operator for `us::Any` 

### Fixed

- `US_ABI_LOCAL` and symbol visibility for gcc \< 4 [\#6](https://github.com/CppMicroServices/CppMicroServices/issues/6)

## [v1.0.0](https://github.com/cppmicroservices/cppmicroservices/tree/v1.0.0) (2013-07-18)

Initial release.

### Fixed

- Build fails on Windows with VS 2012 RC due to CreateMutex [\#5](https://github.com/CppMicroServices/CppMicroServices/issues/5)
- usConfig.h not added to framework on Mac [\#4](https://github.com/CppMicroServices/CppMicroServices/issues/4)
- US\_DEBUG logs even when not in debug mode [\#3](https://github.com/CppMicroServices/CppMicroServices/issues/3)
- Segmentation error after unloading module [\#2](https://github.com/CppMicroServices/CppMicroServices/issues/2)
- Build fails on Ubuntu 12.04 [\#1](https://github.com/CppMicroServices/CppMicroServices/issues/1)

