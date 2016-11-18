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

#include "TestingMacros.h"
#include "TestUtils.h"
#include "TestingConfig.h"
#include "ZipFile.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <array>


using namespace cppmicroservices;

static const std::string manifest_json = R"({
"bundle.symbolic_name" : "main",
"bundle.version" : "0.1.0","
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

  std::string manifest_fpath(tempdir + testing::DIR_SEP + "manifest.json");
  std::ofstream manifest(manifest_fpath);
  if (!manifest.is_open())
  {
    throw std::runtime_error("Couldn't open " + manifest_fpath);
  }
  manifest << manifest_json << std::endl;
  manifest.close();

  std::string rc1dir_path (tempdir + testing::DIR_SEP + "resource1");
  std::string rc2dir_path (tempdir + testing::DIR_SEP + "resource2");
  testing::MakeDirectory(rc1dir_path);
  testing::MakeDirectory(rc2dir_path);
  std::string rc1file_path(rc1dir_path + testing::DIR_SEP + "resource1.txt");
  std::string rc2file_path(rc2dir_path + testing::DIR_SEP + "resource2.txt");
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
  auto create_mock_dll = [](const std::string& tempdir,
                            const std::string& dllname,
                            const std::array<char, 5>& dat)
  {
    std::string dll_path(tempdir + testing::DIR_SEP + dllname);
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
  create_mock_dll(tempdir, "sample.dll", data1);
  create_mock_dll(tempdir, "sample1.dll", data2);
}

/*
* Use resource compiler to create Example.zip with just manifest.json
*/
static void testManifestAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name " << "mybundle";
  cmd << " --out-file " << tempdir << testing::DIR_SEP << "Example.zip";
  cmd << " --manifest-add " << tempdir << testing::DIR_SEP << "manifest.json";
  std::system(cmd.str().c_str());

  ZipFile zip(tempdir + testing::DIR_SEP + "Example.zip");
  US_TEST_CONDITION(zip.size() == 2, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "mybundle/", "Check name of entry 1.")
}

/*
* Use resource compiler to create Example.zip with manifest.json and
* one --res-add option
*/
static void testManifestResAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --out-file Example2.zip ";
  cmd << " --manifest-add manifest.json ";
  cmd << " --res-add resource1/resource1.txt";

  auto cwdir = testing::GetCurrentWorkingDirectory();
  testing::ChangeDirectory(tempdir);
  std::system(cmd.str().c_str());
  testing::ChangeDirectory(cwdir);

  ZipFile zip(tempdir + testing::DIR_SEP + "Example2.zip");
  US_TEST_CONDITION(zip.size() == 4, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip[2].name == "mybundle/resource1/resource1.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip[3].name == "mybundle/resource1/", "Check name of entry 3.")
}

/*
* Use resource compiler to create tomerge.zip with only --res-add option
*/
static void testResAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --out-file tomerge.zip ";
  cmd << " --res-add resource2/resource2.txt";

  auto cwdir = testing::GetCurrentWorkingDirectory();
  testing::ChangeDirectory(tempdir);
  std::system(cmd.str().c_str());
  testing::ChangeDirectory(cwdir);

  ZipFile zip(tempdir + testing::DIR_SEP + "tomerge.zip");
  US_TEST_CONDITION(zip.size() == 3, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "mybundle/resource2/resource2.txt", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip[2].name == "mybundle/resource2/", "Check name of entry 2.")
}

/*
* Use resource compiler to create Example4.zip
* add a manifest, add resource1/resource1.txt using --res-add
* and merge tomerge.zip into Example4.zip using --zip-add
*/
static void testZipAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --manifest-add manifest.json ";
  cmd << " --out-file Example4.zip ";
  cmd << " --res-add resource1/resource1.txt ";
  cmd << " --zip-add tomerge.zip ";

  auto cwdir = testing::GetCurrentWorkingDirectory();
  testing::ChangeDirectory(tempdir);
  std::system(cmd.str().c_str());
  testing::ChangeDirectory(cwdir);

  ZipFile zip(tempdir + testing::DIR_SEP + "Example4.zip");
  US_TEST_CONDITION(zip.size() == 6, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip[2].name == "mybundle/resource1/resource1.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip[3].name == "mybundle/resource1/", "Check name of entry 3.")
  US_TEST_CONDITION(zip[4].name == "mybundle/resource2/resource2.txt", "Check name of entry 4.")
  US_TEST_CONDITION(zip[5].name == "mybundle/resource2/", "Check name of entry 5.")

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
  cmd << " --bundle-file " << tempdir << testing::DIR_SEP << "sample.dll ";
  cmd << " --manifest-add " << tempdir << testing::DIR_SEP << "manifest.json ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "tomerge.zip ";
  std::system(cmd.str().c_str());

  ZipFile zip(tempdir + testing::DIR_SEP + "sample.dll");
  US_TEST_CONDITION(zip.size() == 4, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip[2].name == "mybundle/resource2/resource2.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip[3].name == "mybundle/resource2/", "Check name of entry 3.")
}

/*
* Add two zip-add arguments to a bundle-file.
*/
static void testZipAddTwice(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-file " << tempdir << testing::DIR_SEP << "sample1.dll ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "tomerge.zip ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "Example2.zip ";
  std::system(cmd.str().c_str());

  ZipFile zip(tempdir + testing::DIR_SEP + "sample1.dll");
  US_TEST_CONDITION(zip.size() == 6, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "mybundle/resource2/resource2.txt", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip[2].name == "mybundle/resource2/", "Check name of entry 2.")
  US_TEST_CONDITION(zip[3].name == "mybundle/manifest.json", "Check name of entry 3.")
  US_TEST_CONDITION(zip[4].name == "mybundle/resource1/resource1.txt", "Check name of entry 4.")
  US_TEST_CONDITION(zip[5].name == "mybundle/resource1/", "Check name of entry 5.")
}

/*
* Add a manifest under bundle-name anotherbundle and add two zip blobs using the
* zip-add arguments to a bundle-file.
*/
static void testBundleManifestZipAdd(const std::string& rcbinpath, const std::string& tempdir)
{
  std::ostringstream cmd;
  cmd << rcbinpath;
  cmd << " --bundle-name anotherbundle ";
  cmd << " --manifest-add " << tempdir << testing::DIR_SEP << "manifest.json ";
  cmd << " --bundle-file " << tempdir << testing::DIR_SEP << "sample1.dll ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "tomerge.zip ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "Example2.zip ";
  std::system(cmd.str().c_str());

  ZipFile zip(tempdir + testing::DIR_SEP + "sample1.dll");
  US_TEST_CONDITION(zip.size() == 8, "Check number of entries of zip.")
  US_TEST_CONDITION(zip[0].name == "anotherbundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip[1].name == "anotherbundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip[2].name == "mybundle/resource2/resource2.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip[3].name == "mybundle/", "Check name of entry 3.")
  US_TEST_CONDITION(zip[4].name == "mybundle/resource2/", "Check name of entry 4.")
  US_TEST_CONDITION(zip[5].name == "mybundle/manifest.json", "Check name of entry 5.")
  US_TEST_CONDITION(zip[6].name == "mybundle/resource1/resource1.txt", "Check name of entry 6.")
  US_TEST_CONDITION(zip[7].name == "mybundle/resource1/", "Check name of entry 7.")
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
  cmd << " --bundle-file test2.dll ";
  int ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple bundle-file args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --out-file test1.zip ";
  cmd << " --out-file test2.zip ";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple out-file args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --bundle-name foo ";
  cmd << " --bundle-name bar ";
  cmd << " --bundle-file bundlefile ";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple bundle-name args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --manifest-add m1.json ";
  cmd << " --manifest-add m2.json ";
  cmd << " --bundle-file bundlefile ";
  cmd << " --bundle-name dummy ";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: Multiple manifest-add args")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --manifest-file manifest.json ";
  cmd << " --bundle-name foobundle ";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: --bundle-file or --out-file required")

  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --bundle-name mybundle ";
  cmd << " --bundle-file " << tempdir << testing::DIR_SEP << "sample1.dll ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "tomerge.zip ";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "Example2.zip ";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret == 0, "Failure mode: "
    "--bundle-name arg without either --manifest-add or --res-add is just a warning")

  // Example.zip already contains mybundle/manifest.json
  // Should get an error when we are trying to manifest-add
  // mybundle/manifest.json
  cmd.str("");
  cmd.clear();
  cmd << rcbinpath;
  cmd << " --bundle-name " << "mybundle";
  cmd << " --out-file " << tempdir << testing::DIR_SEP << "Example7.zip";
  cmd << " --zip-add " << tempdir << testing::DIR_SEP << "Example.zip";
  cmd << " --manifest-add " << tempdir << testing::DIR_SEP << "manifest.json";
  ret = std::system(cmd.str().c_str());
  US_TEST_CONDITION(ret != 0, "Failure mode: duplicate manifest.json")
}

/*
* Removes all the stray files and directories. brings back to clean slate
*/
static void makeCleanSlate(const std::string& tempdir)
{
  // remove files created by the tests
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "Example.zip");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "Example2.zip");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "Example4.zip");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "tomerge.zip");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "sample.dll");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "sample1.dll");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "Example5.zip");
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "Example7.zip");

  // remove files created for running the tests
  testing::CheckFileAndRemove(tempdir + testing::DIR_SEP + "manifest.json");
  std::string rc1dir_path = tempdir + testing::DIR_SEP + "resource1";
  std::string rc1file_path(rc1dir_path + testing::DIR_SEP + "resource1.txt");
  std::string rc2dir_path = tempdir + testing::DIR_SEP + "resource2";
  std::string rc2file_path(rc2dir_path + testing::DIR_SEP + "resource2.txt");
  testing::CheckFileAndRemove(rc1file_path.c_str());
  testing::CheckFileAndRemove(rc2file_path.c_str());
  // remove dirs
  if (testing::DirectoryExists(rc1dir_path))
  {
    testing::RemoveDirectory(rc1dir_path);
  }
  if (testing::DirectoryExists(rc2dir_path))
  {
    testing::RemoveDirectory(rc2dir_path);
  }
}

int ResourceCompilerTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ResourceCompilerTest");

  auto rcbinpath = testing::BIN_PATH + testing::DIR_SEP + "usResourceCompiler" + testing::EXE_EXT;
  /*
  * If ResourceCompiler executable is not found, we can't run the tests, we
  * mark it as a failure and exit
  */
  std::ifstream binf(rcbinpath.c_str());
  if (!binf.good())
  {
    US_TEST_FAILED_MSG(<< "Cannot find usResourceCompiler executable.");
    return EXIT_FAILURE;
  }

  auto tempdir = testing::GetTempDirectory();

  US_TEST_NO_EXCEPTION_REQUIRED( makeCleanSlate(tempdir) );

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

  US_TEST_NO_EXCEPTION_REQUIRED( makeCleanSlate(tempdir) );

  US_TEST_END()
}
