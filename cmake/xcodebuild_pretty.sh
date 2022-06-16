#!/bin/bash

# $1 = ${CTEST_BINARY_DIRECTORY}
# $2 = ${CTEST_BUILD_CONFIGURATION}

xcodebuild -project $1/CppMicroServices.xcodeproj build -target ALL_BUILD -configuration $2 -parallelizeTargets -hideShellScriptEnvironment | xcpretty && exit ${PIPESTATUS[0]}
