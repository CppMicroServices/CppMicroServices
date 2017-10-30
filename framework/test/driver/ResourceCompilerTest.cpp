/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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

#include "cppmicroservices/util/FileSystem.h"

#include "TestingMacros.h"
#include "TestingConfig.h"
#include "TestUtils.h"
#include "ZipFile.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <string>
#include <array>

using namespace cppmicroservices;
using namespace cppmicroservices::util;

static const std::string manifest_json = R"({
"bundle.symbolic_name" : "main",
"bundle.version" : "0.1.0",
"bundle.activator" : true
})";

static const std::string resource1_txt = "A sample resource to embed\n"
"Inside the text file resource1.txt";

static const std::string resource2_txt = "A sample resource to embed\n"
"Inside the text file resource2.txt";

/*
* Create a sample directory hierarchy in tempdir
* to perform testing of ResourceCompiler
*/
static void createDirHierarchy(const std::string& tempdir)
{
  /*
   * We create the following directory hierarchy
   * current_dir/
   *    |____ manifest.json
   *    |____ sample.dll
   *    |____ sample1.dll
   *    |____ resource1/
   *    |         |______ resource1.txt
   *    |____ resource2/
   *              |______ resource2.txt
   */

  std::string manifest_fpath(tempdir + "manifest.json");
  std::ofstream manifest(manifest_fpath);
  if (!manifest.is_open())
  {
    throw std::runtime_error("Couldn't open " + manifest_fpath);
  }
  manifest << manifest_json << std::endl;
  manifest.close();

  std::string rc1dir_path (tempdir + "resource1");
  std::string rc2dir_path (tempdir + "resource2");
  MakePath(rc1dir_path);
  MakePath(rc2dir_path);
  std::string rc1file_path(rc1dir_path + DIR_SEP + "resource1.txt");
  std::string rc2file_path(rc2dir_path + DIR_SEP + "resource2.txt");
  std::ofstream rc1file(rc1file_path.c_str());
  std::ofstream rc2file(rc2file_path.c_str());
  if (!rc1file.is_open())
  {
    throw std::runtime_error("Couldn't open " + rc1file_path);
  }
  if (!rc2file.is_open())
  {
    throw std::runtime_error("Couldn't open " + rc2file_path);
  }
  rc1file << resource1_txt << std::endl;
  rc1file.close();
  rc2file << resource2_txt << std::endl;
  rc2file.close();

  // Create 2 binary files filled with random numbers
  // to test bundle-file functionality.
  auto create_mock_dll = [&tempdir](const std::string& dllname,
                                    const std::array<char, 5>& dat)
  {
    std::string dll_path(tempdir + dllname);
    std::ofstream dll(dll_path.c_str());
    if (!dll.is_open())
    {
      throw std::runtime_error("Couldn't open " + dll_path);
    }
    dll.write(dat.data(), dat.size()); //binary write
    dll.close();
  };
  std::array<char, 5> data1 = { {2, 4, 6, 8, 10} };
  std::array<char, 5> data2 = { {1, 2, 3, 4, 5} };
  create_mock_dll("sample.dll", data1);
  create_mock_dll("sample1.dll", data2);
}

/*
* In a vector of strings "entryNames", test if the string "name" exists
*/
static inline void testExists(const std::vector<std::string>& entryNames,
                              const std::string& name)
{
  bool exists = std::find(entryNames.begin(), entryNames.end(), name) != entryNames.end();
  US_TEST_CONDITION(exists, "Check existence of " + name)
}

/*
* Transform the path specified in "path", which may contain a combination of spaces
* and parenthesis, to a path with the spaces and parenthesis escaped with "\".
* If this isn't escaped, test invocation (std::system) with this path results
* in an error that the given file is not found.
*
* E.g. "/tmp/path (space)/rc" will result in a raw "/tmp/path\ \(space\)/rc"
*/
static inline void escapePath(std::string& path)
{
  std::string delimiters("() ");
  std::string insertstr("\\");
  size_t found = path.find_first_of(delimiters);
  while (found != std::string::npos)
  {
    path.insert(found, insertstr);
    found = path.find_first_of(delimiters, found + insertstr.size() + 1);
  }
}

/*
* Use resource compiler to create Example.zip with just manifest.json
*/
static void testManifestAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name " << "mybundle";
  cmd << " --out-file " << tempdir << "Example.zip";
  cmd << " --manifest-add " << tempdir << "manifest.json";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testManifestAdd returns 0")

  ZipFile zip(tempdir + DIR_SEP + "Example.zip");
  US_TEST_CONDITION(zip.size() == 2, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "mybundle/manifest.json");
  testExists(entryNames, "mybundle/");
}

/*
* Use resource compiler to create Example.zip with manifest.json and
* one --res-add option
*
* Working directory is changed temporarily to tempdir because of --res-add option
*/
static void testManifestResAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --out-file Example2.zip ";
  cmd << " --manifest-add manifest.json ";
  cmd << " --res-add resource1/resource1.txt";

  auto cwdir = GetCurrentWorkingDirectory();
  testing::ChangeDirectory(tempdir);
  int ret = std::system(cmd.str().c_str());
  testing::ChangeDirectory(cwdir);
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testManifestResAdd returns 0")

  ZipFile zip(tempdir + "Example2.zip");
  US_TEST_CONDITION(zip.size() == 4, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "mybundle/manifest.json");
  testExists(entryNames, "mybundle/");
  testExists(entryNames, "mybundle/resource1/resource1.txt");
  testExists(entryNames, "mybundle/resource1/");
}

/*
* Use resource compiler to create tomerge.zip with only --res-add option
*
* Working directory is changed temporarily to tempdir because of --res-add option
*/
static void testResAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --out-file tomerge.zip ";
  cmd << " --res-add resource2/resource2.txt";

  auto cwdir = GetCurrentWorkingDirectory();
  testing::ChangeDirectory(tempdir);
  int ret = std::system(cmd.str().c_str());
  testing::ChangeDirectory(cwdir);
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testResAdd returns 0")

  ZipFile zip(tempdir + "tomerge.zip");
  US_TEST_CONDITION(zip.size() == 3, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "mybundle/resource2/resource2.txt");
  testExists(entryNames, "mybundle/");
  testExists(entryNames, "mybundle/resource2/");
}

/*
* Use resource compiler to create Example4.zip
* add a manifest, add resource1/resource1.txt using --res-add
* and merge tomerge.zip into Example4.zip using --zip-add
*
* Working directory is changed temporarily to tempdir because of --res-add option
*/
static void testZipAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --manifest-add manifest.json ";
  cmd << " --out-file Example4.zip ";
  cmd << " --res-add resource1/resource1.txt ";
  cmd << " --zip-add tomerge.zip";

  auto cwdir = GetCurrentWorkingDirectory();
  testing::ChangeDirectory(tempdir);
  int ret = std::system(cmd.str().c_str());
  testing::ChangeDirectory(cwdir);
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testZipAdd returns 0")

  ZipFile zip(tempdir + "Example4.zip");
  US_TEST_CONDITION(zip.size() == 6, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "mybundle/manifest.json");
  testExists(entryNames, "mybundle/");
  testExists(entryNames, "mybundle/resource1/resource1.txt");
  testExists(entryNames, "mybundle/resource1/");
  testExists(entryNames, "mybundle/resource2/resource2.txt");
  testExists(entryNames, "mybundle/resource2/");
}

/*
* Use resource compiler to append a zip-file (using --zip-add)
* to the bundle sample.dll using the -b option and also add a manifest
* while doing so.
*/
static void testZipAddBundle(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --bundle-file " << tempdir << "sample.dll ";
  cmd << " --manifest-add " << tempdir << "manifest.json ";
  cmd << " --zip-add " << tempdir << "tomerge.zip";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testZipAddBundle returns 0")

  ZipFile zip(tempdir + "sample.dll");
  US_TEST_CONDITION(zip.size() == 4, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "mybundle/manifest.json");
  testExists(entryNames, "mybundle/");
  testExists(entryNames, "mybundle/resource2/resource2.txt");
  testExists(entryNames, "mybundle/resource2/");
}

/*
* Add two zip-add arguments to a bundle-file.
*/
static void testZipAddTwice(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-file " << tempdir << "sample1.dll ";
  cmd << " --zip-add " << tempdir  << "tomerge.zip ";
  cmd << " --zip-add " << tempdir  << "Example2.zip";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testZipAddTwice returns 0")

  ZipFile zip(tempdir + "sample1.dll");
  US_TEST_CONDITION(zip.size() == 6, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "mybundle/resource2/resource2.txt");
  testExists(entryNames, "mybundle/");
  testExists(entryNames, "mybundle/resource2/");
  testExists(entryNames, "mybundle/manifest.json");
  testExists(entryNames, "mybundle/resource1/resource1.txt");
  testExists(entryNames, "mybundle/resource1/");
}

/*
* Add a manifest under bundle-name anotherbundle and add two zip blobs using the
* zip-add arguments to a bundle-file.
*/
static void testBundleManifestZipAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd <<  rcbinpath;
  cmd << " --bundle-name anotherbundle ";
  cmd << " --manifest-add " << tempdir << "manifest.json ";
  cmd << " --bundle-file " << tempdir << "sample1.dll ";
  cmd << " --zip-add " << tempdir << "tomerge.zip ";
  cmd << " --zip-add " << tempdir << "Example2.zip";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION_REQUIRED(ret == 0, "Cmdline invocation in testBundleManifestZipAdd "
                                       "returns 0")

  ZipFile zip(tempdir + "sample1.dll");
  US_TEST_CONDITION(zip.size() == 8, "Check number of entries of zip.")

  auto entryNames = zip.getNames();
  testExists(entryNames, "anotherbundle/manifest.json");
  testExists(entryNames, "anotherbundle/");
  testExists(entryNames, "mybundle/resource2/resource2.txt");
  testExists(entryNames, "mybundle/");
  testExists(entryNames, "mybundle/resource2/");
  testExists(entryNames, "mybundle/manifest.json");
  testExists(entryNames, "mybundle/resource1/resource1.txt");
  testExists(entryNames, "mybundle/resource1/");
}

/*
* --help returns exit code 0
*/
static void testHelpReturnsZero(const std::string& rcbinpath)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --help";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret == 0, "help option returns zero")
}

/*
* Test the failure modes of ResourceCompiler command
*/
static void testrcFailureModes(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-file test1.dll ";
  cmd << " --bundle-file test2.dll";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple bundle-file args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --out-file test1.zip ";
  cmd << " --out-file test2.zip";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple out-file args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --bundle-name foo ";
  cmd << " --bundle-name bar ";
  cmd << " --bundle-file bundlefile";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple bundle-name args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --manifest-add m1.json ";
  cmd << " --manifest-add m2.json ";
  cmd << " --bundle-file bundlefile ";
  cmd << " --bundle-name dummy";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple manifest-add args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --manifest-file manifest.json ";
  cmd << " --bundle-name foobundle";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: --bundle-file or --out-file required")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --bundle-file " << tempdir << DIR_SEP << "sample1.dll ";
  cmd << " --zip-add " << tempdir << DIR_SEP << "tomerge.zip ";
  cmd << " --zip-add " << tempdir << DIR_SEP << "Example2.zip";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret == 0,
    "--bundle-name arg without either --manifest-add or --res-add is just a warning")

  // Example.zip already contains mybundle/manifest.json
  // Should get an error when we are trying to manifest-add
  // mybundle/manifest.json
  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --bundle-name " << "mybundle";
  cmd << " --out-file " << tempdir << DIR_SEP << "Example7.zip";
  cmd << " --zip-add " << tempdir << DIR_SEP << "Example.zip";
  cmd << " --manifest-add " << tempdir << DIR_SEP << "manifest.json";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: duplicate manifest.json")
}

// Test escapePath functionality
static void testEscapePath()
{
  // Test escapePath function
  std::string path1("/tmp/path (space)/rc");
  escapePath(path1);
  US_TEST_CONDITION(path1 == "/tmp/path\\ \\(space\\)/rc", "Test escapePath #1")

  std::string path2("/tmp/foo/bar");
  escapePath(path2);
  US_TEST_CONDITION(path2 == "/tmp/foo/bar", "Test escapePath #2")

  std::string path3("/home/travis/CppMicroServices/us builds (Unix make)/bin/usResourceCompiler");
  escapePath(path3);
  US_TEST_CONDITION(path3 == "/home/travis/CppMicroServices/us\\ builds\\ \\(Unix\\ make\\)/bin/usResourceCompiler",
    "Test escapePath #3")
}

int ResourceCompilerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ResourceCompilerTest");

  testEscapePath();

  auto rcbinpath = testing::RCC_PATH;
  /*
  * If ResourceCompiler executable is not found, we can't run the tests, we
  * mark it as a failure and exit
  */
  std::ifstream binf(rcbinpath.c_str());
  if (!binf.good())
  {
    US_TEST_FAILED_MSG(<< "Cannot find usResourceCompiler executable:" << rcbinpath);
    return EXIT_FAILURE;
  }

  testing::TempDir uniqueTempdir = testing::MakeUniqueTempDirectory();
  auto tempdir = uniqueTempdir.Path + DIR_SEP;
#ifndef US_PLATFORM_WINDOWS
  escapePath(tempdir);
  escapePath(rcbinpath);
#endif

  US_TEST_NO_EXCEPTION_REQUIRED( createDirHierarchy(tempdir) );

  US_TEST_NO_EXCEPTION( testManifestAdd(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testManifestResAdd(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testResAdd(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testZipAdd(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testZipAddBundle(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testZipAddTwice(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testBundleManifestZipAdd(rcbinpath, tempdir) );

  US_TEST_NO_EXCEPTION( testHelpReturnsZero(rcbinpath) );

  US_TEST_NO_EXCEPTION( testrcFailureModes(rcbinpath, tempdir) );

  US_TEST_END()
}
