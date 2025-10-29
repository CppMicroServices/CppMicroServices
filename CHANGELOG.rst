Change Log
==========

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <http://keepachangelog.com/>`_
and this project adheres to `Semantic Versioning <http://semver.org/>`_.

`v3.8.9 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.9>`_ (2025-10-31)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.8...3.8.9>`_

Added
-----

Changed
-------
- `[Declarative Services] Enhance bind logs <https://github.com/CppMicroServices/CppMicroServices/pull/1197>`_
- `[Configuration Admin] Ensure synchronous config updates per-config <https://github.com/CppMicroServices/CppMicroServices/pull/1202>`_
- `[Core Framework] Create custom error for invalid bundleContext <https://github.com/CppMicroServices/CppMicroServices/pull/1206>`_

Removed
-------

Deprecated
----------

Fixed
-----
- `[Core Framework] Fix ambiguous overload for c++20 builds <https://github.com/CppMicroServices/CppMicroServices/pull/1195>`_
- `[Core Framework] Fix sporadic test failure with bundle trackers <https://github.com/CppMicroServices/CppMicroServices/pull/1198>`_
- `[Core Framework] Added new value move constructor and move assign operator <https://github.com/CppMicroServices/CppMicroServices/pull/1204>`_
- `[Core Framework] Ensure GetServiceFromFactory is safe <https://github.com/CppMicroServices/CppMicroServices/pull/1210>`_
- `[Core Framework] Solve issue where many threads install the same bundles concurrently <https://github.com/CppMicroServices/CppMicroServices/pull/1209>`_
- `[Core Framework] Silence warning for use of std::atomic_load for use with shared_ptr <https://github.com/CppMicroServices/CppMicroServices/pull/1199>`_
- `[Declarative Services] Ensure that the ComponentConfigurationImpl is stopped and referenceManagers stopTracking before destruction <https://github.com/CppMicroServices/CppMicroServices/pull/1211>`_
- `[Declarative Services] Verify that a singleton's instance is valid before invoking unbind <https://github.com/CppMicroServices/CppMicroServices/pull/1208>`_
- `[Declarative Services] Fix 1207 <https://github.com/CppMicroServices/CppMicroServices/pull/1213>`_

`v3.8.8 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.8>`_ (2025-9-1)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.7...3.8.8>`_

Added
-----
- `[Configuration Admin] Configuration State Logging <https://github.com/CppMicroServices/CppMicroServices/pull/1073>`_
- `[Core Framework] enable windows and linux arm build <https://github.com/CppMicroServices/CppMicroServices/pull/1089>`_
- `[Core Framework] Namespace renaming tool <https://github.com/CppMicroServices/CppMicroServices/pull/1050>`_

Changed
-------
- `[Declarative Services] replace use of std::regex_replace <https://github.com/CppMicroServices/CppMicroServices/pull/1085>`_
- `[Core Framework] Update spdlog to version 1.15.2 <https://github.com/CppMicroServices/CppMicroServices/pull/1115>`_
- `[Declarative Services] new inject-references mechanism <https://github.com/CppMicroServices/CppMicroServices/pull/1112>`_

Removed
-------
- `[Core Framework] Remove httpservice, webconsole, third_party/civetweb, shellservices, shell, third_party/linenoise <https://github.com/CppMicroServices/CppMicroServices/pull/1095>`_
- `[Core Framework] Remove windows-2019 runner <https://github.com/CppMicroServices/CppMicroServices/pull/1104>`_

Deprecated
----------
- `[Core Framework] Remove ubuntu 20.04 from github actions <https://github.com/CppMicroServices/CppMicroServices/pull/1063>`_
- `[Core Framework] Remove unnecessary android workaround <https://github.com/CppMicroServices/CppMicroServices/pull/1096>`_

Fixed
-----
- `[Configuration Admin] concurrent configuration creations for factories race <https://github.com/CppMicroServices/CppMicroServices/pull/1072>`_
- `[Core Framework] Update performance testing <https://github.com/CppMicroServices/CppMicroServices/pull/1081>`_
- `[Declarative Services] Fix binding of prototype services <https://github.com/CppMicroServices/CppMicroServices/pull/1079>`_
- `[Core Framework] Fix benchmark workflvow <https://github.com/CppMicroServices/CppMicroServices/pull/1088>`_
- `[Core Framework] fix space in tempdir for windows <https://github.com/CppMicroServices/CppMicroServices/pull/1093>`_
- `[Declarative Services] export locateService <https://github.com/CppMicroServices/CppMicroServices/pull/1086>`_
- `[Declarative Services] Ensure locateService from within activate returns the ptr to the activated reference <https://github.com/CppMicroServices/CppMicroServices/pull/1092>`_
- `[Core Framework] I161 <https://github.com/CppMicroServices/CppMicroServices/pull/1097>`_
- `[Declarative Services] I0268 <https://github.com/CppMicroServices/CppMicroServices/pull/1100>`_
- `[Core Framework] I0184 <https://github.com/CppMicroServices/CppMicroServices/pull/1102>`_
- `[Declarative Services] RW locks on bundle notifications in the SCR Activator <https://github.com/CppMicroServices/CppMicroServices/pull/1098>`_
- `[Declarative Services] resolve valgrind possibly lost leaks <https://github.com/CppMicroServices/CppMicroServices/pull/1067>`_
- `[Core Framework] Fix Bundle Find Hooks to take into consideration systemBundle <https://github.com/CppMicroServices/CppMicroServices/pull/1107>`_
- `[Declarative Services] Don't unnecessarly reassign the AWS <https://github.com/CppMicroServices/CppMicroServices/pull/1105>`_
- `[Core Framework] Fix changelog formatting <https://github.com/CppMicroServices/CppMicroServices/pull/1180>`_
- `[Declarative Services] Fix clang-tidy warnings in DS, LogService, AWS <https://github.com/CppMicroServices/CppMicroServices/pull/1117>`_
- `[Declarative Services] updates to factory service with Modified method deadlock fix <https://github.com/CppMicroServices/CppMicroServices/pull/1141>`_
- `[Core Framework] Ensure serviceReference object properly manages interface on assignment <https://github.com/CppMicroServices/CppMicroServices/pull/1193>`_

`v3.8.7 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.7>`_ (2025-5-1)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.6...3.8.7>`_

Added
-----
- `[Declarative Services] New Dynamic Targeting mechanism <https://github.com/CppMicroServices/CppMicroServices/pull/1064>`_

Changed
-------

Removed
-------

Deprecated
----------

Fixed
-----
- `[Core Framework] Make ServiceProperties an AnyMap <https://github.com/CppMicroServices/CppMicroServices/pull/1056>`_
- `[Core Framework] Update Tiny Scheme <https://github.com/CppMicroServices/CppMicroServices/pull/1047>`_
- `[Core Framework] Remove libtelnet <https://github.com/CppMicroServices/CppMicroServices/pull/1059>`_
- `[Core Framework] Add missing cstdint includes <https://github.com/CppMicroServices/CppMicroServices/pull/1057>`_
- `[Core Framework] Remove traces of absl <https://github.com/CppMicroServices/CppMicroServices/pull/1058>`_
- `[Core Framework] Resolve first valgrind issue <https://github.com/CppMicroServices/CppMicroServices/pull/1066>`_
- `[Core Framework] getServiceObjects fix to use customDeleter <https://github.com/CppMicroServices/CppMicroServices/pull/1070>`_
- `[Core Framework] Revert conversion of serviceProperties to anymap <https://github.com/CppMicroServices/CppMicroServices/pull/1074>`_

`v3.8.6 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.6>`_ (2025-1-20)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.5...3.8.6>`_

Added
-----
- `[Declarative Services] Allow users to force checking of Activate, Deactivate, and Modified methods at compile time <https://github.com/CppMicroServices/CppMicroServices/pull/1046>`_
- `[Declarative Services] Allow for global namespacing in interface definitions and manifestss <https://github.com/CppMicroServices/CppMicroServices/pull/1048>`_

Changed
-------

Removed
-------

Deprecated
----------

Fixed
-----
- `[Declarative Services] Fix logic for when to require dynamic binding methods <https://github.com/CppMicroServices/CppMicroServices/pull/1045>`_
- `[Core Framework] Update Google Benchmark to v1.9.0 <https://github.com/CppMicroServices/CppMicroServices/pull/1049>`_
- `[Core Framework] Update clang_tidy_complete_code_review.yml <https://github.com/CppMicroServices/CppMicroServices/pull/1052>`_
- `[Declarative Services] Fix reference scoping for factory services <https://github.com/CppMicroServices/CppMicroServices/pull/1054>`_

`v3.8.5 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.5>`_ (2024-12-13)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.4...3.8.5>`_

Added
-----
- `[Configuration Admin] Safe Futures Addition <https://github.com/CppMicroServices/CppMicroServices/pull/1019>`_
- `[Declarative Services] Allow for the presence of both a default constructor and one which takes the properties for the service <https://github.com/CppMicroServices/CppMicroServices/pull/1030>`_
- `[Core Framework] Functionality to retrieve Service Reference from Service <https://github.com/CppMicroServices/CppMicroServices/pull/1044>`_

Changed
-------

Removed
-------

Deprecated
----------

Fixed
-----
- `[Declarative Services] update async deadlock prevention <https://github.com/CppMicroServices/CppMicroServices/pull/1027>`_
- `[Core Framework] Remove linking to library rt from Android build as it is apart of stdc++ <https://github.com/CppMicroServices/CppMicroServices/pull/1038>`_
- `[Core Framework] Create cmake-variants.json <https://github.com/CppMicroServices/CppMicroServices/pull/1033>`_
- `[Log Service] Update LogService class in CppMicroServices <https://github.com/CppMicroServices/CppMicroServices/pull/1009>`_
- `[Declarative Services] Remove deadlock in config notification <https://github.com/CppMicroServices/CppMicroServices/pull/1040>`_
- `[Configuration Admin] Fixes a race condition in Config Admin shutdown <https://github.com/CppMicroServices/CppMicroServices/pull/1043>`_

`v3.8.4 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.4>`_ (2024-6-6)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.3...3.8.4>`_

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
- `[Event Admin] Created Event Admin Interface <https://github.com/CppMicroServices/CppMicroServices/pull/658>`_
- `[Core Framework] Deterministic Builds on all platforms <https://github.com/CppMicroServices/CppMicroServices/pull/996>`_
- `[Configuration Admin] Remove Registration of Factory Configuration object even with valid config <https://github.com/CppMicroServices/CppMicroServices/pull/1006>`_
- `[Declarative Services] handle invalid sevice objects in removeFromBoundServicesCache <https://github.com/CppMicroServices/CppMicroServices/pull/1007>`_
- `[Declarative Services] Release Lock Sooner in SCRRegistryExtension::Remove and SCRRegistryExtension::Clear <https://github.com/CppMicroServices/CppMicroServices/pull/1008>`_
- `[Core Framework] Add Tests to verify support of nested AnyMaps in initializer lists <https://github.com/CppMicroServices/CppMicroServices/pull/1011>`_
- `[Declarative Services] Suppress Valgrind false positives <https://github.com/CppMicroServices/CppMicroServices/pull/1015>`_
- `[Declarative Services] Release lock before calling customer code for config changes <https://github.com/CppMicroServices/CppMicroServices/pull/1012>`_

`v3.8.3 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.3>`_ (2024-4-12)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.2...3.8.3>`_

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
- `[Declarative Services] Add dynamic targeting functionality for factory services to DS <https://github.com/CppMicroServices/CppMicroServices/pull/977>`_
- `[Core Framework] Fixed Clang Tidy warnings <https://github.com/CppMicroServices/CppMicroServices/pull/1003>`_

`v3.8.2 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.2>`_ (2024-3-6)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.1...3.8.2>`_

Added
-----
- `[GithubActions] Updated Github actions to use clang-tidy <https://github.com/CppMicroServices/CppMicroServices/pull/989>`_
- `[GithubActions] Updated Github actions to lint with clang-tidy <https://github.com/CppMicroServices/CppMicroServices/pull/988>`_
- `[GithubActions] Remove Clang-tidy 'modernize-use-trailing-type' <https://github.com/CppMicroServices/CppMicroServices/pull/992>`_

Changed
-------

Removed
-------

Deprecated
----------

Fixed
-----
- `[Declarative Services and Configuration Admin] DS/CA race in Modified() method <https://github.com/CppMicroServices/CppMicroServices/pull/985>`_
- `[Declarative Services and Configuration Admin] Fix deadlock in thread starved environment <https://github.com/CppMicroServices/CppMicroServices/pull/987>`_
- `[Core Framework] Compile CppMicroServices with -noexecstack <https://github.com/CppMicroServices/CppMicroServices/pull/994>`_
- `[Documentation] Configuration listener doc update <https://github.com/CppMicroServices/CppMicroServices/pull/997>`_
- `[GithubActions] Update Codeql version <https://github.com/CppMicroServices/CppMicroServices/pull/998>`_
- `[GithubActions] Update MSVC to silence erroneous warning <https://github.com/CppMicroServices/CppMicroServices/pull/999>`_
- `[Declarative Services] Fix string casting in testUtils <https://github.com/CppMicroServices/CppMicroServices/pull/1000>`_

`v3.8.1 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.1>`_ (2024-2-8)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.8.0...3.8.1>`_

Added
-----
- `[Core Framework] Added Testing section to documentation <https://github.com/CppMicroServices/CppMicroServices/pull/960>`_
- `[Declarative Services] Reenable test for Dictionary <https://github.com/CppMicroServices/CppMicroServices/pull/965>`_

Changed
-------
- `[Core Framework] Remove export of miniz symbols <https://github.com/CppMicroServices/CppMicroServices/pull/966>`_

Removed
-------
- `[Core Framework] TSAN suppression of CCActiveState deadlock <https://github.com/CppMicroServices/CppMicroServices/pull/964>`_

Deprecated
----------

Fixed
-----
- `[Declarative Services] Fix GetBundleContext when using DS <https://github.com/CppMicroServices/CppMicroServices/pull/947>`_
- `[Core Framework] Fix code scan warnings <https://github.com/CppMicroServices/CppMicroServices/pull/969>`_
- `[Core Framework] Fix more code scan warnings <https://github.com/CppMicroServices/CppMicroServices/pull/973>`_
- `[Core Framework] Fixed uninitialized vars warnings <https://github.com/CppMicroServices/CppMicroServices/pull/978>`_
- `[Core Framework] Ensure that Bundle.start() throws after framework has stopped <https://github.com/CppMicroServices/CppMicroServices/pull/979>`_
- `[Core Framework] Ensure safe concurrent destruction of bundles and framework stopping <https://github.com/CppMicroServices/CppMicroServices/pull/983>`_
- `[Core Framework] Fix for concurrent Bundle.start() and framework stop <https://github.com/CppMicroServices/CppMicroServices/pull/990>`_

`v3.8.0 <https://github.com/cppmicroservices/cppmicroservices/tree/3.8.0>`_ (2023-12-06)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.7.6...3.8.0>`_

Added
-----
- `[Core Framework] Guarentee ordering by rank from GetServiceReferences <https://github.com/CppMicroServices/CppMicroServices/pull/943>`_
- `[Core Framework] Add BundleTracker <https://github.com/CppMicroServices/CppMicroServices/pull/726>`_
- `[Core Framework] Initializer list support for AnyMap <https://github.com/CppMicroServices/CppMicroServices/pull/942>`_

Changed
-------
- `[Core Framework] Remove manual reference counting for serviceRegistrations <https://github.com/CppMicroServices/CppMicroServices/pull/841>`_
- `[Core Framework] Ensure ServiceRegistrationU objects are not discarded from call to RegisterService <https://github.com/CppMicroServices/CppMicroServices/pull/863>`_
- `[Core Framework] Update README to reflect correct compiler/OS versions <https://github.com/CppMicroServices/CppMicroServices/pull/862>`_
- `[Declarative Services] Ensure multiple listeners for same factory PID are honored <https://github.com/CppMicroServices/CppMicroServices/pull/865>`_
- `Update github workflows <https://github.com/CppMicroServices/CppMicroServices/pull/902>`_
- `Use custom boost namespace to avoid symbol collision <https://github.com/CppMicroServices/CppMicroServices/pull/929>`_
- `Update 3rd party dependency versions <https://github.com/CppMicroServices/CppMicroServices/pull/930>`_
- `[Core Framework] Guarentee hash of serviceReference is conserved after destruction of serviceRegistrationBase object <https://github.com/CppMicroServices/CppMicroServices/pull/962>`_

Removed
-------
- `Update to allow custom boost namespace and remove absl dependency <https://github.com/CppMicroServices/CppMicroServices/pull/939>`_

Deprecated
----------

Fixed
-----
- `[Core Framework] Data Race Condition fix for Bundles dataStorage location <https://github.com/CppMicroServices/CppMicroServices/pull/845>`_
- `[Core Framework] Remove problematic std::move calls. <https://github.com/CppMicroServices/CppMicroServices/pull/848>`_
- `[Core Framework] Flag Checking <https://github.com/CppMicroServices/CppMicroServices/pull/849>`_
- `[Core Framework] Include cstdint in FileSystem.cpp <https://github.com/CppMicroServices/CppMicroServices/pull/850>`_
- `[Core Framework] Fix code scanning alerts <https://github.com/CppMicroServices/CppMicroServices/pull/861>`_
- `[Config Admin, Declarative Services] Fix code scanning alerts <https://github.com/CppMicroServices/CppMicroServices/pull/866>`_
- `[Declarative Services] Fix race condition when addint SCRExtensionRegistry <https://github.com/CppMicroServices/CppMicroServices/pull/870>`_
- `[Core Framework] Recoup performance losses <https://github.com/CppMicroServices/CppMicroServices/pull/869>`_
- `[Core Framework] Recoup performance losses <https://github.com/CppMicroServices/CppMicroServices/pull/874>`_
- `[Core Framework] BundleContextTest.NoSegfaultWithServiceFactory sporadic failure fix <https://github.com/CppMicroServices/CppMicroServices/pull/876>`_
- `[Core Framework] Allow char const* properties in LDAPFilters <https://github.com/CppMicroServices/CppMicroServices/pull/877>`_
- `[Core Framework] Reformat hpp FileSystem <https://github.com/CppMicroServices/CppMicroServices/pull/880>`_
- `[Core Framework] Disable incorrect TSAN warnings <https://github.com/CppMicroServices/CppMicroServices/pull/878>`_
- `[Core Framework] Fix potential deadlock in ServiceTracker <https://github.com/CppMicroServices/CppMicroServices/pull/883>`_
- `[Core Framework] Update tests to remove unnecessary globals <https://github.com/CppMicroServices/CppMicroServices/pull/875>`_
- `[Core Framework] Fix serviceTracker deadlock on close() <https://github.com/CppMicroServices/CppMicroServices/pull/922>`_
- `[Core Framework] Update github workflows <https://github.com/CppMicroServices/CppMicroServices/pull/916>`_
- `[Core Framework] Remove unused variable and add missing include <https://github.com/CppMicroServices/CppMicroServices/pull/932>`_
- `[Declarative Services] Fix redundant bundle validation checks <https://github.com/CppMicroServices/CppMicroServices/pull/921>`_
- `[Core Framework] Fix serviceTracker deadlock <https://github.com/CppMicroServices/CppMicroServices/pull/915>`_

`v3.7.6 <https://github.com/cppmicroservices/cppmicroservices/tree/3.7.6>`_ (2023-04-25)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.7.5...3.7.6>`_

Added
-----
- `[Declarative Services] Add benchmark test infrastructure to DS <https://github.com/CppMicroServices/CppMicroServices/pull/813>`_
- `[Core Framework] Make nested JSON queries using LDAP build-time configurable <https://github.com/CppMicroServices/CppMicroServices/pull/811>`_
- `[Core Framework] Support nested JSON queries using LDAP <https://github.com/CppMicroServices/CppMicroServices/pull/794>`_

Changed
-------
- `Upgrade GitHub Actions to use Ubuntu 22.04 and remove use of Ubuntu 18.04 <https://github.com/CppMicroServices/CppMicroServices/pull/810>`_

Removed
-------

Deprecated
----------

Fixed
-----
- `[Core Framework] clang-tidy improvement for CMakeResourceDependencies <https://github.com/CppMicroServices/CppMicroServices/pull/812>`_
- `[Core Framework] GetService performance micro-optimizations <https://github.com/CppMicroServices/CppMicroServices/pull/833>`_
- `[Declarative Services] Fix sporadic crash caused by concurrent access to ComponentMgrImpl vector <https://github.com/CppMicroServices/CppMicroServices/pull/834>`_

`v3.7.5 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.7.5>`_ (2023-03-14)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.7.4...v3.7.5>`_

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
- `[Core Framework] Performance improvements <https://github.com/CppMicroServices/CppMicroServices/pull/728>`_
- `[Core Framework] Fix undefined behavior <https://github.com/CppMicroServices/CppMicroServices/pull/777>`_
- `[Declarative Services] Fix race with Declarative Services service object construction <https://github.com/CppMicroServices/CppMicroServices/pull/801>`_
- `[Core Framework] RegisterService performance improvement <https://github.com/CppMicroServices/CppMicroServices/pull/808>`_


`v3.7.4 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.7.4>`_ (2022-11-02)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.7.2...v3.7.4>`_

Added
-----
- `Support arm64 on macOS <https://github.com/CppMicroServices/CppMicroServices/pull/778>`_

Changed
-------
- Code formatting, no functional changes:
    - `updated formatting - clang-fromat ran on all files <https://github.com/CppMicroServices/CppMicroServices/pull/759>`_
    - `Clang-format git hook pre-commit enforcement <https://github.com/CppMicroServices/CppMicroServices/pull/760>`_
    - `clang-format ran on all files <https://github.com/CppMicroServices/CppMicroServices/pull/766>`_
- `[Core Framework] Upgrade jsoncpp <https://github.com/CppMicroServices/CppMicroServices/pull/773>`_

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


`(UNRELEASED) v3.7.3 <https://github.com/cppmicroservices/cppmicroservices/tree/13ca108641c1960539cdaed10bcc39ae9a46b7a6>`_ (2022-08-29)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.7.2...13ca108641c1960539cdaed10bcc39ae9a46b7a6>`_

Added
-----
- `Add MSVC analysis to project <https://github.com/CppMicroServices/CppMicroServices/pull/685>`_

Changed
-------
- `[Declarative Services] Improve error message that is generated when an appropriate constructor isn't found for the Service Instance. <https://github.com/CppMicroServices/CppMicroServices/pull/724>`_
- `[Configuration Admin] Remove automatic config object creation <https://github.com/CppMicroServices/CppMicroServices/pull/717>`_
- `Updated CI to use macos-12 <https://github.com/CppMicroServices/CppMicroServices/pull/711>`_
- `Update CXX_STANDARD to 17 for doc <https://github.com/CppMicroServices/CppMicroServices/pull/705>`_
- `[Core Framework] Upgrade miniz to v3.0 <https://github.com/CppMicroServices/CppMicroServices/pull/688>`_
- `[Core Framework] Remove manual ref counting for BundleResource <https://github.com/CppMicroServices/CppMicroServices/pull/695>`_
- `Add ignore for 3rdparty code for MSVC code analysis <https://github.com/CppMicroServices/CppMicroServices/pull/692>`_
- `[Core Framework/Declarative Services] Add log messages when shared library loading throws an exception <https://github.com/CppMicroServices/CppMicroServices/pull/690>`_

Removed
-------

Deprecated
----------

Fixed
-----
- `[Declarative Services] Factory Configuration Bug Fix <https://github.com/CppMicroServices/CppMicroServices/pull/731>`_
- `[Configuration Admin] Fix race that results in a missed config updated event <https://github.com/CppMicroServices/CppMicroServices/pull/727>`_
- `[Core Framework] Fixed sporadic race conditions during framework shutdown <https://github.com/CppMicroServices/CppMicroServices/pull/725>`_
- `[Core Framework] Ensure that the ServiceTracker::GetTrackingCount() method returns -1 if the tracker has been opened and then closed. <https://github.com/CppMicroServices/CppMicroServices/pull/714>`_
- `Added missing include for <thread> <https://github.com/CppMicroServices/CppMicroServices/pull/721>`_
- `[Declarative Services] BugFix when creating instance name for factory components <https://github.com/CppMicroServices/CppMicroServices/pull/720>`_
- `[Configuration Admin] Fix race in ConfigurationNotifier::NotifyAllListeners() <https://github.com/CppMicroServices/CppMicroServices/pull/715>`_
- `[Configuration Admin] Fix deadlock <https://github.com/CppMicroServices/CppMicroServices/pull/651>`_
- `[Core Framework] Improve performance of LDAP matching <https://github.com/CppMicroServices/CppMicroServices/pull/704>`_
- `[Core Framework] Fix CFRlogger accessviolation <https://github.com/CppMicroServices/CppMicroServices/pull/706>`_
- `Cleaned up some security warnings regarding 'noexcept' <https://github.com/CppMicroServices/CppMicroServices/pull/700>`_
- `[Configuration Admin] Multiple services and factory services in bundle dependent on same configuration pid <https://github.com/CppMicroServices/CppMicroServices/pull/698>`_
- `Disable code signing for bundle with no c++ code <https://github.com/CppMicroServices/CppMicroServices/pull/697>`_
- `Fix compilation issue for arm macOS native compilation <https://github.com/CppMicroServices/CppMicroServices/pull/696>`_
- `[Core Framework] Add file handle leak test <https://github.com/CppMicroServices/CppMicroServices/pull/693>`_
- `[Configuration Admin] ListConfigurations fix for empty configuration objects. <https://github.com/CppMicroServices/CppMicroServices/pull/682>`_



`v3.7.2 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.7.2>`_ (2022-06-16)
---------------------------------------------------------------------------------------------------------

`Full Changelog <https://github.com/cppmicroservices/cppmicroservices/compare/v3.6.0...v3.7.2>`_

General Note
------------

The last time CppMicroServices was upgraded to a new version on GitHub was two years ago. We think
it will not be useful to include every change since then; below we have captured all of the
relevant changes between `v3.6.0 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.6.0>`_
and `v3.7.2 <https://github.com/cppmicroservices/cppmicroservices/tree/v3.7.2>`_.

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
- `Switch project to c++17 <https://github.com/CppMicroServices/CppMicroServices/pull/654>`_
- `Upgraded to CMake 3.17 <https://github.com/CppMicroServices/CppMicroServices/pull/655>`_
- `[Core Framework] Switch code to use std::string_view instead of abseil <https://github.com/CppMicroServices/CppMicroServices/pull/657>`_
- `[Core Framework] Integrate LogService core framework and add more detail to exception messages <https://github.com/CppMicroServices/CppMicroServices/pull/680>`_

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