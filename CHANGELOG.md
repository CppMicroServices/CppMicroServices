# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/) 
and this project adheres to [Semantic Versioning](http://semver.org/).

This is **work in progress**

## [Unreleased](https://github.com/cppmicroservices/cppmicroservices/tree/HEAD)

[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v2.1.1...development)

### Added

for new features

### Changed

for changes in existing functionality

### Deprecated

for oncoe-stable features removed in upcoming releases

### Removed

for deprecated features removed in upcoming releases

### Fixed

for any bug fixes

### Security

to invite users to upgrade in case of vulnerabilities

**Implemented enhancements:**

- Headers with "\_p.h" suffix do not get resolved in Xcode for automatic-tracking of counterparts [\#93](https://github.com/CppMicroServices/CppMicroServices/issues/93)
- remove autoload feature [\#75](https://github.com/CppMicroServices/CppMicroServices/issues/75)
- Support LDAP queries against bundle meta-data [\#53](https://github.com/CppMicroServices/CppMicroServices/issues/53)
- Switch to using OSGi nomenclature [\#46](https://github.com/CppMicroServices/CppMicroServices/issues/46)
- Change System Bundle ID to 0 [\#45](https://github.com/CppMicroServices/CppMicroServices/issues/45)
- Re-organize public header files [\#43](https://github.com/CppMicroServices/CppMicroServices/issues/43)
- Implement framework properties [\#42](https://github.com/CppMicroServices/CppMicroServices/issues/42)
- Make ModuleInfo private [\#41](https://github.com/CppMicroServices/CppMicroServices/issues/41)
- Use managed pointers for services, modules and the framework [\#38](https://github.com/CppMicroServices/CppMicroServices/issues/38)
- Use C++11 features [\#35](https://github.com/CppMicroServices/CppMicroServices/issues/35)
- Output exception details \(if available\) for troubleshooting [\#27](https://github.com/CppMicroServices/CppMicroServices/issues/27)
- Extend module life-cycle states [\#25](https://github.com/CppMicroServices/CppMicroServices/issues/25)
- Make the usage of US\_DECLARE\_SERVICE\_INTERFACE macro optional [\#24](https://github.com/CppMicroServices/CppMicroServices/issues/24)
- Proper coverity scan support [\#16](https://github.com/CppMicroServices/CppMicroServices/issues/16)
- Enhance the project structure to support sub-projects [\#14](https://github.com/CppMicroServices/CppMicroServices/issues/14)
- Use JSON format in Any::ToString\(\) [\#12](https://github.com/CppMicroServices/CppMicroServices/issues/12)
- 53 LDAP query bundle metadata [\#76](https://github.com/CppMicroServices/CppMicroServices/pull/76) ([kevinleeMW](https://github.com/kevinleeMW))

**Fixed bugs:**

- ResourceCompiler fails to embed manifest when using --bundle-file and --manifest-add together [\#124](https://github.com/CppMicroServices/CppMicroServices/issues/124)
- Trying to stop the bundle returned by GetBundle\(0\) causes a crash [\#121](https://github.com/CppMicroServices/CppMicroServices/issues/121)
- Crash when trying to access bundle context. [\#110](https://github.com/CppMicroServices/CppMicroServices/issues/110)
- Framework fails to activate the bundle inside an executable if the bundle name is not "main" [\#88](https://github.com/CppMicroServices/CppMicroServices/issues/88)
- Resource compiler leaves a temporary zip file behind in append mode on unix platforms [\#86](https://github.com/CppMicroServices/CppMicroServices/issues/86)
- Mac OS X build failure using Makefiles [\#81](https://github.com/CppMicroServices/CppMicroServices/issues/81)
- usShell throws exception when built with US\_BUILD\_SHARED\_LIBS=OFF [\#61](https://github.com/CppMicroServices/CppMicroServices/issues/61)
- unit test failures on Mac OS X 10.10.4/Xcode 6.4 [\#56](https://github.com/CppMicroServices/CppMicroServices/issues/56)
- build error on linux 64-bit \(development branch\) [\#49](https://github.com/CppMicroServices/CppMicroServices/issues/49)
- Add FrameworkListener and improve Framework logging [\#40](https://github.com/CppMicroServices/CppMicroServices/issues/40)
- CppMicroServices fails to build using GCC 4.9.3 on Linux 64-bit  [\#34](https://github.com/CppMicroServices/CppMicroServices/issues/34)
- usUtils.cpp - Crash can occur if FormatMessage\(...\) fails [\#33](https://github.com/CppMicroServices/CppMicroServices/issues/33)
- Possible multithreading dead locks in CppMicroServices [\#32](https://github.com/CppMicroServices/CppMicroServices/issues/32)
- Using US\_DECLARE\_SERVICE\_INTERFACE with Qt does not work [\#19](https://github.com/CppMicroServices/CppMicroServices/issues/19)
- If aut-loading of a library fails, the error message is not propery handled [\#17](https://github.com/CppMicroServices/CppMicroServices/issues/17)
- Framework shutdown race [\#117](https://github.com/CppMicroServices/CppMicroServices/pull/117) ([saschazelzer](https://github.com/saschazelzer))
- gcc build errors [\#50](https://github.com/CppMicroServices/CppMicroServices/pull/50) ([saschazelzer](https://github.com/saschazelzer))

**Closed issues:**

- Fix linker warning on OS X [\#137](https://github.com/CppMicroServices/CppMicroServices/issues/137)
- Auto install of executable bundles leads to exceptions that are hidden [\#131](https://github.com/CppMicroServices/CppMicroServices/issues/131)
- Build failure on Mac due to namespace confusion [\#128](https://github.com/CppMicroServices/CppMicroServices/issues/128)
- Mac OS X and iOS code signing failures [\#114](https://github.com/CppMicroServices/CppMicroServices/issues/114)
- Link failure due to unused argument -pthread when using Clang and Makefiles [\#107](https://github.com/CppMicroServices/CppMicroServices/issues/107)
- support building for Android [\#106](https://github.com/CppMicroServices/CppMicroServices/issues/106)
- Some public/installed headers appear to be unintentional, and some \#include non-installed headers [\#67](https://github.com/CppMicroServices/CppMicroServices/issues/67)
- US\_DECLARE\_SERVICE\_INTERFACE needs a namespace in the inline function definition. [\#58](https://github.com/CppMicroServices/CppMicroServices/issues/58)
- usResourceCompiler needs a verbose option [\#55](https://github.com/CppMicroServices/CppMicroServices/issues/55)
- Rename 'REGISTERED' macro [\#47](https://github.com/CppMicroServices/CppMicroServices/issues/47)
- Move auto-loading facility into a new module  [\#39](https://github.com/CppMicroServices/CppMicroServices/issues/39)
- Make log levels configurable [\#22](https://github.com/CppMicroServices/CppMicroServices/issues/22)
- Rework static module system [\#21](https://github.com/CppMicroServices/CppMicroServices/issues/21)
- Example event listener uses wrong property type [\#20](https://github.com/CppMicroServices/CppMicroServices/issues/20)
- Success of the usModuleResourceTest depends on line ending convention [\#18](https://github.com/CppMicroServices/CppMicroServices/issues/18)
- Support resource access from modules without loading them [\#15](https://github.com/CppMicroServices/CppMicroServices/issues/15)
- Support last modified time for embedded resources [\#13](https://github.com/CppMicroServices/CppMicroServices/issues/13)

**Merged pull requests:**

- Fix a bug in resource compiler and resubmit lost commit [\#141](https://github.com/CppMicroServices/CppMicroServices/pull/141) ([karthikreddy09](https://github.com/karthikreddy09))
- Fixed linker warning on OS X [\#140](https://github.com/CppMicroServices/CppMicroServices/pull/140) ([karthikreddy09](https://github.com/karthikreddy09))
- Test CI with OS X [\#136](https://github.com/CppMicroServices/CppMicroServices/pull/136) ([karthikreddy09](https://github.com/karthikreddy09))
- Fix manifest files for examples [\#133](https://github.com/CppMicroServices/CppMicroServices/pull/133) ([karthikreddy09](https://github.com/karthikreddy09))
- 131 auto install and bundle registry exceptions [\#132](https://github.com/CppMicroServices/CppMicroServices/pull/132) ([saschazelzer](https://github.com/saschazelzer))
- Move "\#include \<cxxabi.h\>" outside the "cppmicroservices" namespace [\#129](https://github.com/CppMicroServices/CppMicroServices/pull/129) ([kevinleeMW](https://github.com/kevinleeMW))
- Move virtual methods to BundlePrivate and fix the system bundle. [\#125](https://github.com/CppMicroServices/CppMicroServices/pull/125) ([saschazelzer](https://github.com/saschazelzer))
- General cleanup [\#122](https://github.com/CppMicroServices/CppMicroServices/pull/122) ([saschazelzer](https://github.com/saschazelzer))
- prevent “\_FORTIFY\_SOURCE” redefinition [\#120](https://github.com/CppMicroServices/CppMicroServices/pull/120) ([mpkh](https://github.com/mpkh))
- Use consistent catch blocks and exception detail messages. [\#119](https://github.com/CppMicroServices/CppMicroServices/pull/119) ([saschazelzer](https://github.com/saschazelzer))
- 43 header reorg [\#118](https://github.com/CppMicroServices/CppMicroServices/pull/118) ([kevinleeMW](https://github.com/kevinleeMW))
- Fix for reporting file write error in resource compiler [\#116](https://github.com/CppMicroServices/CppMicroServices/pull/116) ([karthikreddy09](https://github.com/karthikreddy09))
- 106 - Add support for building for Android \(std::to\_string\) [\#111](https://github.com/CppMicroServices/CppMicroServices/pull/111) ([kevinleeMW](https://github.com/kevinleeMW))
- 88 auto install embedded bundles from executable [\#109](https://github.com/CppMicroServices/CppMicroServices/pull/109) ([karthikreddy09](https://github.com/karthikreddy09))
- Add sysroot to fix Makefile build on OS X \#81 [\#108](https://github.com/CppMicroServices/CppMicroServices/pull/108) ([fnannizzi](https://github.com/fnannizzi))
- 25 bundle lifecycle [\#105](https://github.com/CppMicroServices/CppMicroServices/pull/105) ([saschazelzer](https://github.com/saschazelzer))
- Framework Listener and logging support [\#96](https://github.com/CppMicroServices/CppMicroServices/pull/96) ([jeffdiclemente](https://github.com/jeffdiclemente))
- 75 remove autoinstall [\#92](https://github.com/CppMicroServices/CppMicroServices/pull/92) ([karthikreddy09](https://github.com/karthikreddy09))
- Fixed resource compiler to only call us\_tempfile function when not in append mode. [\#87](https://github.com/CppMicroServices/CppMicroServices/pull/87) ([karthikreddy09](https://github.com/karthikreddy09))
- fix mac os x test failures [\#82](https://github.com/CppMicroServices/CppMicroServices/pull/82) ([jeffdiclemente](https://github.com/jeffdiclemente))
- Clean header dependencies [\#68](https://github.com/CppMicroServices/CppMicroServices/pull/68) ([mrlegowatch](https://github.com/mrlegowatch))
- Rename mc to context [\#65](https://github.com/CppMicroServices/CppMicroServices/pull/65) ([mrlegowatch](https://github.com/mrlegowatch))
- 55 resource compiler port to cpp [\#63](https://github.com/CppMicroServices/CppMicroServices/pull/63) ([karthikreddy09](https://github.com/karthikreddy09))
- Use shared\_ptr\<Bundle\> [\#60](https://github.com/CppMicroServices/CppMicroServices/pull/60) ([jeffdiclemente](https://github.com/jeffdiclemente))
- 58 fix us declare service interface [\#59](https://github.com/CppMicroServices/CppMicroServices/pull/59) ([karthikreddy09](https://github.com/karthikreddy09))
- 38 smart pointers for services [\#57](https://github.com/CppMicroServices/CppMicroServices/pull/57) ([karthikreddy09](https://github.com/karthikreddy09))
- Fix Windows only bug in GetLastErrorStr\(\) [\#52](https://github.com/CppMicroServices/CppMicroServices/pull/52) ([jeffdiclemente](https://github.com/jeffdiclemente))
- Implement framework properties [\#51](https://github.com/CppMicroServices/CppMicroServices/pull/51) ([jeffdiclemente](https://github.com/jeffdiclemente))
- use osgi nomenclature + system bundle ID is 0 [\#48](https://github.com/CppMicroServices/CppMicroServices/pull/48) ([jeffdiclemente](https://github.com/jeffdiclemente))
- Remove static globals [\#31](https://github.com/CppMicroServices/CppMicroServices/pull/31) ([jeffdiclemente](https://github.com/jeffdiclemente))

## [v2.1.1](https://github.com/cppmicroservices/cppmicroservices/tree/v2.1.1) (2014-01-22)
[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v2.1.0...v2.1.1)

**Closed issues:**

- Resource compiler not found error [\#11](https://github.com/CppMicroServices/CppMicroServices/issues/11)

## [v2.1.0](https://github.com/cppmicroservices/cppmicroservices/tree/v2.1.0) (2014-01-11)
[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v2.0.0...v2.1.0)

**Implemented enhancements:**

- Use the version number from CMakeLists.txt in the manifest file [\#10](https://github.com/CppMicroServices/CppMicroServices/issues/10)
- Build fails on Mac OS Mavericks with 10.9 SDK [\#7](https://github.com/CppMicroServices/CppMicroServices/issues/7)

**Fixed bugs:**

- Comparison of service listener objects is buggy on VS 2008 [\#9](https://github.com/CppMicroServices/CppMicroServices/issues/9)
- Service listener memory leak [\#8](https://github.com/CppMicroServices/CppMicroServices/issues/8)

## [v2.0.0](https://github.com/cppmicroservices/cppmicroservices/tree/v2.0.0) (2013-12-23)
[Full Changelog](https://github.com/cppmicroservices/cppmicroservices/compare/v1.0.0...v2.0.0)

**Closed issues:**

- US\_ABI\_LOCAL and symbol visibility for gcc \< 4 [\#6](https://github.com/CppMicroServices/CppMicroServices/issues/6)

## [v1.0.0](https://github.com/cppmicroservices/cppmicroservices/tree/v1.0.0) (2013-07-18)
**Closed issues:**

- Build fails on Windows with VS 2012 RC due to CreateMutex [\#5](https://github.com/CppMicroServices/CppMicroServices/issues/5)
- usConfig.h not added to framework on Mac [\#4](https://github.com/CppMicroServices/CppMicroServices/issues/4)
- US\_DEBUG logs even when not in debug mode [\#3](https://github.com/CppMicroServices/CppMicroServices/issues/3)
- Segmentation error after unloading module [\#2](https://github.com/CppMicroServices/CppMicroServices/issues/2)
- Build fails on Ubuntu 12.04 [\#1](https://github.com/CppMicroServices/CppMicroServices/issues/1)

