# Diff Report: `development` branch vs sandbox

Comparing:
- **This repo (development branch):** `/home/tcormack/dev/fixBoostLinkage/CppMicroServices`
- **Sandbox:** `/mathworks/devel/sandbox/tcormack/forDiffWithGit/3p/sources/CppMicroServices`

---

## Files only in sandbox (MathWorks build/packaging metadata)

- `3P_VERSION`
- `3p_version.json`
- `buildinfo.sh`
- `makefile_3p.mk`
- `NO_LOCAL_DERIVED`
- `unifiedWindowsStaticDestructionWorkaround.patch`
- `framework/examples/` (directory)
- `framework/test/driver/` (directory)
- `framework/test/bundles/libBWithStatic/resources/{dynamic.txt,res.txt}`
- `framework/test/bundles/libBWithStatic/resources_static/{res.txt,static.txt}`
- `framework/test/bundles/libRWithResources/resources/{foo.txt,special_chars.dummy.txt}`

## Files only in this repo

- `build/` (build artifact)
- `.vscode/`
- `DIFF_REPORT.md` (this file)
- `third_party/boost/include/boost/cregex.hpp`

---

## Substantive Code Differences

### 1. CMakeLists.txt (root) — Boost linkage logic

Sandbox adds `Boost_USE_STATIC_LIBS ON` on Windows and uses `target_link_directories` + `Boost::boost` directly. This repo tries `boost::boost` then falls back to `Boost::boost`.

```diff
--- this repo (development)
+++ sandbox
@@ -391,9 +392,8 @@
+    if(WIN32)
+        set(Boost_USE_STATIC_LIBS ON)
+    endif()
     ...
-    if(TARGET boost::boost)
-      set(_us_boost_target boost::boost)
-    else()
-      set(_us_boost_target Boost::boost)
+    if(WIN32)
+        target_link_directories(nowide::nowide INTERFACE ${Boost_LIBRARY_DIRS})
     endif()
-    target_link_libraries(nowide::nowide INTERFACE ${_us_boost_target})
+    target_link_libraries(nowide::nowide INTERFACE Boost::boost)
```

---

### 2. cmake/usFunctionAddResources.cmake — missing `_rc_env_cmd` init

```diff
--- this repo
+++ sandbox
@@ -101,0 +102,1 @@
+    set(_rc_env_cmd)
```

Same in `cmake/usFunctionEmbedResources.cmake`:

```diff
--- this repo
+++ sandbox
@@ -133,0 +134,1 @@
+    set(_rc_env_cmd)
```

---

### 3. compendium/ConfigurationAdmin/src/CMakeLists.txt — indentation

```diff
--- this repo
+++ sandbox
@@ -51,5 +51,5 @@
-usFunctionBoostPath(BOOST_SYSTEM ${US_USE_SYSTEM_BOOST} CPPMS_SOURCE_DIR ${CppMicroServices_SOURCE_DIR} BOOST_DIR ${BOOST_INCLUDEDIR})
+  usFunctionBoostPath(BOOST_SYSTEM ${US_USE_SYSTEM_BOOST} CPPMS_SOURCE_DIR ${CppMicroServices_SOURCE_DIR} BOOST_DIR ${BOOST_INCLUDEDIR})
 
-# There are warnings in the boost asio headers which are flagged as errors. Include the boost
-# asio headers as system headers to ignore these warnings and not treat them as errors.
-include_directories(SYSTEM ${_boost_library})
+  # There are warnings in the boost asio headers which are flagged as errors. Include the boost
+  # asio headers as system headers to ignore these warnings and not treat them as errors.
+  include_directories(SYSTEM ${_boost_library})
```

---

### 4. compendium/DeclarativeServices/test/gtest/TestMultipleRequiredConfigurations.cpp

Sandbox uses `std::atomic<int>` spin-barrier instead of `ConcurrencyTestUtil.hpp`'s `Barrier` class.

```diff
--- this repo
+++ sandbox
@@ -26,2 +27,2 @@
+#include <atomic>
 ...
-#include "../../../DeclarativeServices/test/gtest/ConcurrencyTestUtil.hpp"
@@ -80,1 +80,1 @@
-     * Test to expose a race in ConfigurationManager::Initialize().
+     * Stress test to expose a race in ConfigurationManager::Initialize().
@@ -85,3 +85,3 @@
-     * those two calls, GetProperties() throws. The fix (try/catch in
-     * Initialize()) ensures the component eventually recovers when the
-     * config is re-created.
+     * those two calls, GetProperties() throws. The catch-all in Initialize()
+     * swallows the exception and returns, leaving the component permanently
+     * unsatisfied with no recovery path.
@@ -89,3 +89,4 @@
-     * Strategy: concurrently start the bundle (which triggers async
-     * component initialization) and remove/re-create pid1. Fewer iterations
-     * than a brute-force stress test, but enough to exercise the window.
+     * To trigger: concurrently start the bundle (which triggers async
+     * component initialization) and remove/re-create pid1.
+     *
+     * Run with --gtest_repeat=-1 to loop until failure.
@@ -99,1 +100,1 @@
-        constexpr auto POLL_INTERVAL = std::chrono::milliseconds(50);
+        constexpr auto POLL_INTERVAL = std::chrono::milliseconds(5);
@@ -102,0 +104,1 @@
+            std::cout << "repetition: " << i << std::endl;
@@ -119,1 +121,2 @@
-            Barrier barrier(2);
+            // Spin-barrier so both threads release at the same time.
+            std::atomic<int> barrier{0};
@@ -124,1 +127,2 @@
-                barrier.Wait();
+                barrier.fetch_add(1, std::memory_order_release);
+                while (barrier.load(std::memory_order_acquire) < 2) {}
@@ -131,1 +135,2 @@
-                barrier.Wait();
+                barrier.fetch_add(1, std::memory_order_release);
+                while (barrier.load(std::memory_order_acquire) < 2) {}
```

---

### 5. third_party/benchmark/CMakeLists.txt

```diff
--- this repo
+++ sandbox
@@ -133,0 +134,1 @@
+include(CheckLibraryExists)
```

---

### 6. .github/workflows/build_and_test_nix.yml

```diff
--- this repo
+++ sandbox
@@ -89,1 +89,1 @@
-      if: ${{startsWith(matrix.os, 'ubuntu')}}
+      if: ${{matrix.os == 'ubuntu-22.04'}}
@@ -121,1 +121,1 @@
-      if: ${{startsWith(matrix.os, 'ubuntu')}}
+      if: ${{matrix.os == 'ubuntu-22.04'}}
```

(Also trailing space differences on `env:` lines)

---

## Version Bumps (sandbox is ahead)

| File | This repo | Sandbox |
|------|-----------|---------|
| `VERSION` | 3.8.12 | 3.8.13 |
| `compendium/ConfigurationAdmin/CMakeLists.txt` | 1.3.15 | 1.3.16 |
| `compendium/DeclarativeServices/CMakeLists.txt` | 1.5.19 | 1.5.20 |

---

## CHANGELOG.rst

Sandbox has the full v3.8.13 release notes block added at the top, plus minor date corrections:
- v3.8.2: "2024-3-6" -> "2024-2-26"
- v3.8.1: "2024-2-8" -> "2024-1-26"

---

## MW WORKAROUND-guarded changes (excluded from analysis)

These are all guarded by `// BEGIN MW WORKAROUND` ... `// END MW WORKAROUND` comments and exist only in the sandbox:

- `framework/include/cppmicroservices/Framework.h` — `isProcessTerminating()`/`setProcessTerminating()` API
- `framework/include/cppmicroservices/detail/WaitCondition.h` — heap-allocated condition variable
- `framework/include/cppmicroservices/detail/ServiceTracker.hpp` — early-return on termination
- `framework/src/util/Framework.cpp` — implementation of termination API
- `framework/src/util/FrameworkFactory.cpp` — early-returns on termination/already-stopped
- `framework/src/util/Utils.cpp` — `shutdown::` namespace implementation
- `framework/src/util/Utils.h` — `shutdown::` namespace declaration
- `compendium/DeclarativeServices/src/manager/states/CCActiveState.cpp` — commented-out instance check (leak checker workaround)

---

## Whitespace/formatting only (no semantic change)

- `framework/include/cppmicroservices/ServiceReference.h` — tabs vs 8-space indent
- `compendium/DeclarativeServices/src/manager/ConfigurationNotifier.hpp` — extra leading space before `};`
- `framework/test/bench/CMakeLists.txt` — line ending differences (identical content)
- `framework/test/bench/ServiceRegistryTest.cpp` — line ending differences (identical content)
- `framework/test/gtest/BundleHooksTest.cpp` — trailing whitespace on one line
- `compendium/ConfigurationAdmin/src/ConfigurationAdminImpl.cpp` — trailing newline
- `compendium/ConfigurationAdmin/src/ConfigurationAdminImpl.hpp` — trailing newline
- `compendium/test_bundles/CMakeLists.txt` — blank line removals
- `compendium/DeclarativeServices/CMakeLists.txt` — blank line removal
- `framework/include/cppmicroservices/ServiceProperties.h` — added "Deprecated. Use AnyMap instead." comment
