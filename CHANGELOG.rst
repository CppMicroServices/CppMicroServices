Change Log
==========

All notable changes to this project branch will be documented in this file.

The format is based on `Keep a Changelog <http://keepachangelog.com/>`_
and this project adheres to `Semantic Versioning <http://semver.org/>`_.

`Unreleased C++14-branch based on 3.6.4 <https://github.com/cppmicroservices/cppmicroservices/tree/c++14-compliant>`_
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/CppMicroServices/CppMicroServices/compare/c++14-compliant...v3.6.4>`_

General Note
------------

This branch is intended to continue to provide bugfixes and improvements while staying compliant to C++14.

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

`v3.6.4 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.6.4>`_ (2025-08-01)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/CppMicroServices/CppMicroServices/compare/v3.6.4...v3.6.3>`_

General Note
------------

This entry in the change log captures the relevant changes that were made between v3.6.3 and v3.6.4.
This version contains all changes between v3.7.6 and v3.8.7 plus additional changes from `development` after the v3.8.7 release.

The trigger for this release were vulnerabilities in third party software. The main focus of this release was on framework, so quite some changes especially of compendium/DeclarativeServices have not been included.

Some cherry picked changes needed to be adapted for C++14 compliance since they were using nested namespaces, inline variable definition, `std::shared_mutex` and `std::filesystem` from C++17 standard.

Added
-----
- `[Core Framework] Added Testing section to documentation <https://github.com/CppMicroServices/CppMicroServices/pull/1142>`_
- `[GithubActions] Updated Github actions to use clang-tidy <https://github.com/CppMicroServices/CppMicroServices/pull/1145>`_
- `[GithubActions] Updated Github actions to lint with clang-tidy <https://github.com/CppMicroServices/CppMicroServices/pull/1146>`_
- `[GithubActions] Remove Clang-tidy 'modernize-use-trailing-type' <https://github.com/CppMicroServices/CppMicroServices/pull/1159>`_
- `[Configuration Admin] Safe Futures Addition <https://github.com/CppMicroServices/CppMicroServices/pull/1162>`_
- `[Core Framework] Functionality to retrieve Service Reference from Service <https://github.com/CppMicroServices/CppMicroServices/pull/1157>`_
- `[Declarative Services] Allow for global namespacing in interface definitions and manifests <https://github.com/CppMicroServices/CppMicroServices/pull/1158>`_
- `[GithubActions] enable Windows and Linux ARM builds <https://github.com/CppMicroServices/CppMicroServices/pull/1171>`_

Changed
-------
- `[Core Framework] Guarentee hash of serviceReference is conserved after destruction of serviceRegistrationBase object <https://github.com/CppMicroServices/CppMicroServices/pull/1128>`_
- `[GithubActions] Use ubuntu-22.04 <https://github.com/CppMicroServices/CppMicroServices/pull/1181>`_
- `[Core Framework] Improve ResourceCompiler code <https://github.com/CppMicroServices/CppMicroServices/pull/1169>`_
- `[Core Framework] Moved google benchmark to v1.9.0 <https://github.com/CppMicroServices/CppMicroServices/pull/1123>`_
- `[Core Framework] Update spdlog to version 1.15.2 <https://github.com/CppMicroServices/CppMicroServices/pull/1122>`_
- `[Core Framework] Update Tiny Scheme to 1.42 <https://github.com/CppMicroServices/CppMicroServices/pull/1124>`_

Removed
-------
- `[Core Framework] Remove export of miniz symbols <https://github.com/CppMicroServices/CppMicroServices/pull/1129>`_
- `[Core Framework] Remove bundled libtelnet code <https://github.com/CppMicroServices/CppMicroServices/pull/1125>`_
- `[Core Framework] TSAN suppression of CCActiveState deadlock <https://github.com/CppMicroServices/CppMicroServices/pull/1143>`_
- `[Core Framework] Remove export of miniz symbols <https://github.com/CppMicroServices/CppMicroServices/pull/966>`_
- `[Core Framework] Removed unnecessary android only workaround <https://github.com/CppMicroServices/CppMicroServices/pull/1168>`_
- `[Core Framework] Remove httpservice, webconsole, third_party/civetweb, shellservices, shell, third_party/linenoise <https://github.com/CppMicroServices/CppMicroServices/pull/1127>`_
- `[GithubActions] Remove use of windows-2019 runner <https://github.com/CppMicroServices/CppMicroServices/pull/1175>`_

Deprecated
----------

Fixed
-----
- `[Declarative Services] Fix GetBundleContext when using DS <https://github.com/CppMicroServices/CppMicroServices/pull/1130>`_
- `[Core Framework] Fix code scan warnings <https://github.com/CppMicroServices/CppMicroServices/pull/1139>`_
- `[Core Framework] Fix more code scan warnings <https://github.com/CppMicroServices/CppMicroServices/pull/1140>`_
- `[Core Framework] Ensure that Bundle.start() throws after framework has stopped <https://github.com/CppMicroServices/CppMicroServices/pull/1144>`_
- `[Core Framework] Ensure safe concurrent destruction of bundles and framework stopping <https://github.com/CppMicroServices/CppMicroServices/pull/1131>`_
- `[Declarative Services and Configuration Admin] Fix deadlock in thread starved environment <https://github.com/CppMicroServices/CppMicroServices/pull/1152>`_
- `[Core Framework] FixUpdate spdlog to version 1.15.2 for concurrent Bundle.start() and framework stop <https://github.com/CppMicroServices/CppMicroServices/pull/1189>`_
- `[Core Framework] Compile CppMicroServices with -noexecstack <https://github.com/CppMicroServices/CppMicroServices/pull/1133>`_
- `[GithubActions] Update Codeql version <https://github.com/CppMicroServices/CppMicroServices/pull/1160>`_
- `[GithubActions] Update MSVC to silence erroneous warning <https://github.com/CppMicroServices/CppMicroServices/pull/1147>`_
- `[Declarative Services] Fix string casting in testUtils <https://github.com/CppMicroServices/CppMicroServices/pull/1182>`_
- `[Core Framework] Deterministic Builds on all platforms <https://github.com/CppMicroServices/CppMicroServices/pull/1148>`_
- `[Core Framework] Add Tests to verify support of nested AnyMaps in initializer lists <https://github.com/CppMicroServices/CppMicroServices/pull/1149>`_
- `[Core Framework] Remove linking to library rt from Android build as it is apart of stdc++ <https://github.com/CppMicroServices/CppMicroServices/pull/1154>`_
- `[Core Framework] Create cmake-variants.json <https://github.com/CppMicroServices/CppMicroServices/pull/1155>`_
- `[Log Service] Update LogService class in CppMicroServices <https://github.com/CppMicroServices/CppMicroServices/pull/1163>`_
- `[Core Framework] Update clang_tidy_complete_code_review.yml <https://github.com/CppMicroServices/CppMicroServices/pull/1164>`_
- `[Core Framework] Add missing cstdint includes <https://github.com/CppMicroServices/CppMicroServices/pull/1134>`_
- `[Core Framework] getServiceObjects fix to use customDeleter <https://github.com/CppMicroServices/CppMicroServices/pull/1166>`_
- `[Core Framework] Space in tempdir for Windows <https://github.com/CppMicroServices/CppMicroServices/pull/1167>`_
- `[Core Framework] Fix ResourceCompiler <https://github.com/CppMicroServices/CppMicroServices/pull/1170>`_
- `[Core Framework] Resolve valgrind 'possibly lost' leaks <https://github.com/CppMicroServices/CppMicroServices/pull/1179>`_
- `[Core Framework] Updated schema to correctly check that the symbolic_name follows c identifier rules <https://github.com/CppMicroServices/CppMicroServices/pull/1135>`_
- `[Core Framework] Fix Bundle Find Hooks to take into consideration systemBundle <https://github.com/CppMicroServices/CppMicroServices/pull/1136>`_

`v3.6.3 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.6.3>`_ (2023-10-30)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/CppMicroServices/CppMicroServices/compare/v3.6.3...v3.6.2>`_

General Note
------------

This entry in the change log captures the relevant changes that were made between v3.6.2 and v3.6.3.
This version contains all changes between v3.7.5 and v3.7.6 plus additional changes from `development` after the v3.7.6 release.
Additionally there have been 3rd-party dependency updates and some compiler warning/include statement fixes.


Added
-----
- `[Declarative Services] Add benchmark test infrastructure to DS <https://github.com/CppMicroServices/CppMicroServices/pull/813>`_
- `[Core Framework] Make nested JSON queries using LDAP build-time configurable <https://github.com/CppMicroServices/CppMicroServices/pull/811>`_
- `[Core Framework] Support nested JSON queries using LDAP <https://github.com/CppMicroServices/CppMicroServices/pull/794>`_
- `[Declarative Services] Support for multiple cardinality for service references <https://github.com/CppMicroServices/CppMicroServices/pull/871>`_
- `[Core Framework] GetServiceReferences ordering guarantee - Fix #937 <https://github.com/CppMicroServices/CppMicroServices/pull/943>`_
- `[Core Framework] Initializer list support for AnyMap <https://github.com/CppMicroServices/CppMicroServices/pull/942>`_

Changed
-------
- `Upgrade GitHub Actions to use Ubuntu 22.04 and remove use of Ubuntu 18.04 <https://github.com/CppMicroServices/CppMicroServices/pull/810>`_
- `[Core Framework] Remove manual reference counting from ServiceRegistrationBasePrivate and ServiceReferenceBasePrivate <https://github.com/CppMicroServices/CppMicroServices/pull/841>`_
- `Update 3rd party components <https://github.com/CppMicroServices/CppMicroServices/pull/927>`_
    - update absl to Abseil LTS 20230125.3
    - update googletest to release 1.14.0
    - update spdlog to version 1.12.0
    - update boost nowide to latest commit from standalone branch (conforming to Release v11.3.0)
- `Allow building with an external Boost installation <https://github.com/CppMicroServices/CppMicroServices/pull/944>`_

Removed
-------

Deprecated
----------

Fixed
-----
- `[Core Framework] clang-tidy improvement for CMakeResourceDependencies <https://github.com/CppMicroServices/CppMicroServices/pull/812>`_
- `[Core Framework] GetService performance micro-optimizations <https://github.com/CppMicroServices/CppMicroServices/pull/833>`_
- `[Declarative Services] Fix sporadic crash caused by concurrent access to ComponentMgrImpl vector <https://github.com/CppMicroServices/CppMicroServices/pull/834>`_
- `[Core Framework] Data Race Condition fix for Bundles dataStorage location <https://github.com/CppMicroServices/CppMicroServices/pull/845>`_
- `[Core Framework] Fix undefined behavior in LDAPExpr::Trim <https://github.com/CppMicroServices/CppMicroServices/pull/835>`_
- `[Declarative Services] Ensure multiple listeners for the same factory PID are honoured by ConfigurationNotifier::AnyListenersForPid <https://github.com/CppMicroServices/CppMicroServices/pull/865>`_
- `[Declarative Services] Fix race condition when concurrently adding to SCRExtensionRegistry <https://github.com/CppMicroServices/CppMicroServices/pull/870>`_
- `[Core Framework] Fix #489 service trackers using filters aren't notified of a service if it's service properties are constructed with a const char* value <https://github.com/CppMicroServices/CppMicroServices/pull/877>`_
- `[Core Framework] Fix #920 serviceTracker segfault on concurrent tracker.Close() and framework.Stop() <https://github.com/CppMicroServices/CppMicroServices/pull/922>`_
- `[Declarative Services] Fix #901 Redundant Bundle Validation checks <https://github.com/CppMicroServices/CppMicroServices/pull/901>`_
- `[Core Framework] Fix #913 ServiceTracker deadlock <https://github.com/CppMicroServices/CppMicroServices/pull/915>`_


`v3.6.2 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.6.2>`_ (2023-03-16)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/CppMicroServices/CppMicroServices/compare/v3.6.1...v3.6.2>`_

General Note
------------

This entry in the change log captures the relevant changes that were made between v3.6.1 and v3.6.2.
This version contains all changes between v3.7.2 and v3.7.5 except setting CXX_STANDARD to 17 for the documentation.
Additionally there have been 3rd-party dependency updates and some compiler warning fixes not in v3.7.4.

Added
-----

Changed
-------
- Code formatting, no functional changes:
    - `updated formatting - clang-fromat ran on all files <https://github.com/CppMicroServices/CppMicroServices/pull/759>`_
    - `Clang-format git hook pre-commit enforcement <https://github.com/CppMicroServices/CppMicroServices/pull/760>`_
    - `clang-format ran on all files <https://github.com/CppMicroServices/CppMicroServices/pull/766>`_
- `Update spdlog to v1.11.0 and its bundled fmt to latest master <https://github.com/CppMicroServices/CppMicroServices/pull/789>`_
- `Update miniz to v3.0.2 <https://github.com/CppMicroServices/CppMicroServices/pull/788>`_
- `Update gtest to v1.13.0 to fix compiler warnings with gcc-12 <https://github.com/CppMicroServices/CppMicroServices/pull/803>`_
- `[Core Framework] Improve performance of LDAP matching - C++14 variant <https://github.com/CppMicroServices/CppMicroServices/pull/793>`_
- `[Declarative Services] Improve error message that is generated when an appropriate constructor isn't found for the Service Instance. <https://github.com/CppMicroServices/CppMicroServices/pull/724>`_
- `[Configuration Admin] Remove automatic config object creation <https://github.com/CppMicroServices/CppMicroServices/pull/717>`_
- `Updated CI to use macos-12 <https://github.com/CppMicroServices/CppMicroServices/pull/711>`_
- `[Core Framework] Remove manual ref counting for BundleResource <https://github.com/CppMicroServices/CppMicroServices/pull/695>`_
- `Add ignore for 3rdparty code for MSVC code analysis <https://github.com/CppMicroServices/CppMicroServices/pull/692>`_
- `[Core Framework/Declarative Services] Add log messages when shared library loading throws an exception <https://github.com/CppMicroServices/CppMicroServices/pull/690>`_
- `Only run certain BundleContextTests if threading support is enabled <https://github.com/CppMicroServices/CppMicroServices/pull/669>`_

Removed
-------

Deprecated
----------

Fixed
-----
- `[Configuration Admin] Fix deadlock in ConfigurationAdminImpl::RemoveConfigurations <https://github.com/CppMicroServices/CppMicroServices/pull/748>`_
- `[Configuration Admin] configurations using the same pid are not updated properly <https://github.com/CppMicroServices/CppMicroServices/pull/754>`_
- `[Declarative Services] Ensure ~SCRBundleExtension does not throw <https://github.com/CppMicroServices/CppMicroServices/pull/761>`_
- `Fix broken static build configurations on macOS <https://github.com/CppMicroServices/CppMicroServices/pull/774>`_
- `[Core Framework] Performance improvements <https://github.com/CppMicroServices/CppMicroServices/pull/728>`_
- `[Core Framework] Fix undefined behavior <https://github.com/CppMicroServices/CppMicroServices/pull/777>`_
- `[Declarative Services] Fix race with Declarative Services service object construction <https://github.com/CppMicroServices/CppMicroServices/pull/801>`_
- `[Core Framework] RegisterService performance improvement <https://github.com/CppMicroServices/CppMicroServices/pull/808>`_
- `[Core Framework] Ensure that the ServiceTracker::GetTrackingCount() method returns -1 if the tracker has been opened and then closed. <https://github.com/CppMicroServices/CppMicroServices/pull/714>`_
- `[Declarative Services] BugFix when creating instance name for factory components <https://github.com/CppMicroServices/CppMicroServices/pull/720>`_
- `[Configuration Admin] Fix race in ConfigurationNotifier::NotifyAllListeners() <https://github.com/CppMicroServices/CppMicroServices/pull/715>`_
- `[Core Framework] Improve performance of LDAP matching <https://github.com/CppMicroServices/CppMicroServices/pull/704>`_
- `[Core Framework] Fix CFRlogger accessviolation <https://github.com/CppMicroServices/CppMicroServices/pull/706>`_
- `Cleaned up some security warnings regarding 'noexcept' <https://github.com/CppMicroServices/CppMicroServices/pull/700>`_
- `[Configuration Admin] Multiple services and factory services in bundle dependent on same configuration pid <https://github.com/CppMicroServices/CppMicroServices/pull/698>`_
- `Disable code signing for bundle with no c++ code <https://github.com/CppMicroServices/CppMicroServices/pull/697>`_
- `Fix compilation issue for arm macOS native compilation <https://github.com/CppMicroServices/CppMicroServices/pull/696>`_
- `[Core Framework] Add file handle leak test <https://github.com/CppMicroServices/CppMicroServices/pull/693>`_
- `[ConfigurationAdmin] Factory Configuration Bug Fix <https://github.com/CppMicroServices/CppMicroServices/pull/731>`_
- `[Configuration Admin] Fix race that results in missed config updated event <https://github.com/CppMicroServices/CppMicroServices/pull/727>`_
- `[Core Framework] Fix sporadic race conditions during framework shutdown <https://github.com/CppMicroServices/CppMicroServices/pull/725>`_
- `[Configuration Admin] ListConfigurations fix for empty configuration objects. <https://github.com/CppMicroServices/CppMicroServices/pull/682>`_
- `[Configuration Admin] Fix deadlock and double update. <https://github.com/CppMicroServices/CppMicroServices/pull/651>`_
- `[SCRCodeGen] Fixed compiler warning for gcc-12 <https://github.com/CppMicroServices/CppMicroServices/pull/803>`_


`v3.6.1 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.6.1>`_ (2022-12-06)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.6.0...v3.6.1>`_

General Note
------------

This version contains all changes from v3.7.2 except the ones introducing C++17 functionality and removing abseil.
Additionally there have been 3rd-party dependency updates and some compiler warning fixes not in v3.7.2.
Same as for v3.7.2, this list does not include every change since v3.6.0, but only the relevant changes.

Added
-----
- `[Log Service] LogService Implementation <https://github.com/CppMicroServices/CppMicroServices/pull/499>`_
- `[Declarative Services] Added thread pool to DS <https://github.com/CppMicroServices/CppMicroServices/pull/509>`_
- `[Core Framework] Anymap erase and compare <https://github.com/CppMicroServices/CppMicroServices/pull/540>`_
- `Provide updated manifest.json schema in repo <https://github.com/CppMicroServices/CppMicroServices/pull/583>`_
- `[Configuration Admin/Declarative Services] Configadmin ds integration <https://github.com/CppMicroServices/CppMicroServices/pull/512>`_
- GitHub Actions used as CI/CD solution
- `[AsyncWorkService] Added AsyncWorkService interface to CppMicroServices <https://github.com/CppMicroServices/CppMicroServices/pull/598>`_
- GitHub Actions runs workflow for clang build with ASAN, TSAN, and UBSAN enabled
- `[AsyncWorkService] AsyncWorkService DS Integration <https://github.com/CppMicroServices/CppMicroServices/pull/599>`_
- `[AsyncWorkService] AsyncWorkService ConfigAdmin Integration <https://github.com/CppMicroServices/CppMicroServices/pull/620>`_
- `[Security/Core Framework] bundle validation mechanism <https://github.com/CppMicroServices/CppMicroServices/pull/630>`_

Changed
-------

- [Core Framework] Migrated all test driver tests to gtest
- `Optimize the string creation in us_service_interface_iid<void>() <https://github.com/CppMicroServices/CppMicroServices/pull/523>`_
- `[Core Framework] Improve the performance of removing service listeners <https://github.com/CppMicroServices/CppMicroServices/pull/626>`_
- `[Declarative Services] Reduce possibility for symbol conflicts with autogenerated DS files <https://github.com/CppMicroServices/CppMicroServices/pull/647>`_
- `Upgraded to CMake 3.17 <https://github.com/CppMicroServices/CppMicroServices/pull/655>`_
- `[Core Framework] Integrate LogService core framework and add more detail to exception messages <https://github.com/CppMicroServices/CppMicroServices/pull/680>`_
- `Updated spdlog to v1.10.0 <https://github.com/CppMicroServices/CppMicroServices/pull/722>`_
- `Updated jsoncpp to 1.9.5 <https://github.com/CppMicroServices/CppMicroServices/pull/722>`_
- `Updated abseil to latest LTS release 20220623.1 <https://github.com/CppMicroServices/CppMicroServices/pull/732>`_

Removed
-------

- `[Core Framework] Remove Bundle Threads <https://github.com/CppMicroServices/CppMicroServices/pull/533>`_
- TravisCI and Appveyor removed as CI/CD solution

Deprecated
----------

Fixed
-----

- `[Core Framework] Fix ServiceTracker race <https://github.com/CppMicroServices/CppMicroServices/pull/518>`_
- `[Core Framework] Made ToJSON for strings do proper escaping of special characters <https://github.com/CppMicroServices/CppMicroServices/pull/527>`_
- `[Core Framework] Fix issues with line endings in BundleResourceTest.cpp test <https://github.com/CppMicroServices/CppMicroServices/pull/531>`_
- `Fix #301 so TSAN buidls work on Linux <https://github.com/CppMicroServices/CppMicroServices/pull/537>`_
- `[Configuration Admin] ConfigAdmin remove notification when configuration object is created <https://github.com/CppMicroServices/CppMicroServices/pull/539>`_
- `[Configuration Admin] Fix TestConcurrentBindUnbind error <https://github.com/CppMicroServices/CppMicroServices/commit/61f8a8a150741feaacbadb18ee53720a211dcc31>`_
- `[Core Framework] Fix ServiceTracker race <https://github.com/CppMicroServices/CppMicroServices/pull/558>`_
- `[Configuration Admin] RemoveBoundServicesCache fix <https://github.com/CppMicroServices/CppMicroServices/commit/93b4cbfe570942dd282fc53749586426e31de82b>`_
- `[Declarative Services] Prevent nullptr from being passed to service constructors <https://github.com/CppMicroServices/CppMicroServices/pull/572>`_
- `[Declarative Services] fix segfault when service class ctor throws <https://github.com/CppMicroServices/CppMicroServices/pull/586>`_
- `[Core Framework] Fix bug when an AddingService method returns nullptr <https://github.com/CppMicroServices/CppMicroServices/pull/613>`_
- `[Configuration Admin] Fix deadlock in ConfigAdmin Update, UpdateIfDifferent, and Remove <https://github.com/CppMicroServices/CppMicroServices/pull/612>`_
- `[Configuration Admin] Fix ConfigurationAdmin ListConfigurations and Fix Race Conditions in DS <https://github.com/CppMicroServices/CppMicroServices/commit/630ef502035801603cd30334de10b591b77e5716>`_
- `[Resource Compimler] Allow ResourceCompiler to accept Unicode Command-line arguments <https://github.com/CppMicroServices/CppMicroServices/pull/624>`_
- `[Core Framework] service tracker doesn't track services after a close and then open <https://github.com/CppMicroServices/CppMicroServices/pull/627>`_
- `Fix warning suppression leakage to non-CppMicroServices code inside translation units <https://github.com/CppMicroServices/CppMicroServices/commit/25e11cdabfc1f46da79139e15ff06e9825fa305a>`_
- `[Core Framework] Fix leak in ServiceListeners.cpp <https://github.com/CppMicroServices/CppMicroServices/pull/639>`_
- `[Core Framework] Ensure that any BundleContext functions do not segfault if the bundle context is invalid <https://github.com/CppMicroServices/CppMicroServices/pull/656>`_
- `Fixed issue where DS/CA do not build if US_BUILD_TESTING isn't ON <https://github.com/CppMicroServices/CppMicroServices/pull/661>`_
- `Fixed build issues when building in "getting_started" dir <https://github.com/CppMicroServices/CppMicroServices/pull/662>`_
- `[Core Framework] Fix check-then-act-race in GetServiceFromFactory <https://github.com/CppMicroServices/CppMicroServices/pull/664>`_
- `[Core Framework] Fix bug in FindResources() for data-only bundles <https://github.com/CppMicroServices/CppMicroServices/pull/667>`_
- `[Core Framework] Fix check-then-act race for BundleContext <https://github.com/CppMicroServices/CppMicroServices/pull/665>`_
- `[Declarative Services] Fix inheritance for ComponentException <https://github.com/CppMicroServices/CppMicroServices/pull/676>`_
- `[Core Framework] Add --max-threads flag for core framework mem test <https://github.com/CppMicroServices/CppMicroServices/pull/679>`_
- `[Core Framework] Fix leaked file handle <https://github.com/CppMicroServices/CppMicroServices/pull/681>`_
- `Fixed missing include statements for newer libstdc++ headers <https://github.com/CppMicroServices/CppMicroServices/pull/722>`_


`v3.6.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.6.0>`_ (2020-08-13)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.5.0...v3.6.0>`_

Added
-----

- `[Declarative Services] Support dynamic policy reference option <https://github.com/CppMicroServices/CppMicroServices/pull/482>`_
- `Added initial implementation of Configuration Admin <https://github.com/CppMicroServices/CppMicroServices/pull/487>`_

Changed
-------

- `BundleContext::InstallBundles <https://github.com/CppMicroServices/CppMicroServices/pull/481>`_

Removed
-------

Deprecated
----------

Fixed
-----

- `Fixed data race in BundleRegistry::Install <https://github.com/CppMicroServices/CppMicroServices/pull/484>`_
- `Fixed race condition in Declarative Services <https://github.com/CppMicroServices/CppMicroServices/pull/492>`_
- `Removed gtest dependency when not building the tests <https://github.com/CppMicroServices/CppMicroServices/pull/486>`_


`v3.5.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.5.0>`_ (2020-07-04)
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
