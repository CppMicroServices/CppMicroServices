/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include <usSharedLibrary.h>

#include "usTestingMacros.h"
#include "usTestingConfig.h"

#include <cstdlib>
#include <stdexcept>

US_USE_NAMESPACE

int usSharedLibraryTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("SharedLibraryTest");

#ifdef US_PLATFORM_WINDOWS
  const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
  const std::string LIB_PREFIX = "";
  const std::string LIB_SUFFIX = ".dll";
  const char PATH_SEPARATOR = '\\';
#elif defined(US_PLATFORM_APPLE)
  const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
  const std::string LIB_PREFIX = "lib";
  const std::string LIB_SUFFIX = ".dylib";
  const char PATH_SEPARATOR = '/';
#else
  const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
  const std::string LIB_PREFIX = "lib";
  const std::string LIB_SUFFIX = ".so";
  const char PATH_SEPARATOR = '/';

#endif

  const std::string libAFilePath = LIB_PATH + PATH_SEPARATOR + LIB_PREFIX + "TestModuleA" + LIB_SUFFIX;
  SharedLibrary lib1(libAFilePath);
  US_TEST_CONDITION(lib1.GetFilePath() == libAFilePath, "Absolute file path")
  US_TEST_CONDITION(lib1.GetLibraryPath() == LIB_PATH, "Library path")
  US_TEST_CONDITION(lib1.GetName() == "TestModuleA", "Name")
  US_TEST_CONDITION(lib1.GetPrefix() == LIB_PREFIX, "Prefix")
  US_TEST_CONDITION(lib1.GetSuffix() == LIB_SUFFIX, "Suffix")
  lib1.SetName("bla");
  US_TEST_CONDITION(lib1.GetName() == "TestModuleA", "Name after SetName()")
  lib1.SetLibraryPath("bla");
  US_TEST_CONDITION(lib1.GetLibraryPath() == LIB_PATH, "Library path after SetLibraryPath()")
  lib1.SetPrefix("bla");
  US_TEST_CONDITION(lib1.GetPrefix() == LIB_PREFIX, "Prefix after SetPrefix()")
  lib1.SetSuffix("bla");
  US_TEST_CONDITION(lib1.GetSuffix() == LIB_SUFFIX, "Suffix after SetSuffix()")
  US_TEST_CONDITION(lib1.GetFilePath() == libAFilePath, "File path after setters")

  lib1.SetFilePath("bla");
  US_TEST_CONDITION(lib1.GetFilePath() == "bla", "Invalid file path")
  US_TEST_CONDITION(lib1.GetLibraryPath().empty(), "Empty lib path")
  US_TEST_CONDITION(lib1.GetName() == "bla", "Invalid file name")
  US_TEST_CONDITION(lib1.GetPrefix() == LIB_PREFIX, "Invalid prefix")
  US_TEST_CONDITION(lib1.GetSuffix() == LIB_SUFFIX, "Invalid suffix")

  US_TEST_FOR_EXCEPTION(std::runtime_error, lib1.Load())
  US_TEST_CONDITION(lib1.IsLoaded() == false, "Is loaded")
  US_TEST_CONDITION(lib1.GetHandle() == NULL, "Handle")

  lib1.SetFilePath(libAFilePath);
  lib1.Load();
  US_TEST_CONDITION(lib1.IsLoaded() == true, "Is loaded")
  US_TEST_CONDITION(lib1.GetHandle() != NULL, "Handle")
  US_TEST_FOR_EXCEPTION(std::logic_error, lib1.Load())

  lib1.SetFilePath("bla");
  US_TEST_CONDITION(lib1.GetFilePath() == libAFilePath, "File path")
  lib1.Unload();


  SharedLibrary lib2(LIB_PATH, "TestModuleA");
  US_TEST_CONDITION(lib2.GetFilePath() == libAFilePath, "File path")
  lib2.SetPrefix("");
  US_TEST_CONDITION(lib2.GetPrefix().empty(), "Lib prefix")
  US_TEST_CONDITION(lib2.GetFilePath() == LIB_PATH + PATH_SEPARATOR + "TestModuleA" + LIB_SUFFIX, "File path")

  SharedLibrary lib3 = lib2;
  US_TEST_CONDITION(lib3.GetFilePath() == lib2.GetFilePath(), "Compare file path")
  lib3.SetPrefix(LIB_PREFIX);
  US_TEST_CONDITION(lib3.GetFilePath() == libAFilePath, "Compare file path")
  lib3.Load();
  US_TEST_CONDITION(lib3.IsLoaded(), "lib3 loaded")
  US_TEST_CONDITION(!lib2.IsLoaded(), "lib2 not loaded")
  lib1 = lib3;
  US_TEST_FOR_EXCEPTION(std::logic_error, lib1.Load())
  lib2.SetPrefix(LIB_PREFIX);
  lib2.Load();

  lib3.Unload();
  US_TEST_CONDITION(!lib3.IsLoaded(), "lib3 unloaded")
  US_TEST_CONDITION(!lib1.IsLoaded(), "lib3 unloaded")
  lib2.Unload();
  lib1.Unload();

  US_TEST_END()
}
