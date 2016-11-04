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
#include "miniz.h"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

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
* @brief A simple container for storing entries of a zipped archive
*/
class ZipFile
{
public:
  /*
  * @brief Struct which represents an entry in the zip archive.
  */
  struct EntryInfo
  {
    enum class EntryType { FILE, DIRECTORY };

    std::string name;
    EntryType type;
    mz_uint64 compressedSize;
    mz_uint64 uncompressedSize;
    mz_uint32 crc32;
  };

  /*
  * @brief Read a zip archive and fill entries
  * @param filename is the path of the filename
  * @throw std::runtime exception if zip archive couldn't be read
  */
  ZipFile(std::string filename)
  {
    mz_zip_archive ziparchive;
    memset(&ziparchive, 0, sizeof(mz_zip_archive));
    if (!mz_zip_reader_init_file(&ziparchive, filename.c_str(), 0))
    {
      throw std::runtime_error("Could not read zip archive file " + filename);
    }

    mz_uint numindices = mz_zip_reader_get_num_files(&ziparchive);
    for (mz_uint index = 0; index < numindices; ++index)
    {
      mz_zip_archive_file_stat filestat;
      EntryInfo entry;
      mz_zip_reader_file_stat(&ziparchive, index, &filestat);

      entry.name = filestat.m_filename;
      entry.type = mz_zip_reader_is_file_a_directory(&ziparchive, index) ? \
        EntryInfo::EntryType::DIRECTORY : EntryInfo::EntryType::FILE;
      entry.compressedSize = filestat.m_comp_size;
      entry.uncompressedSize = filestat.m_uncomp_size;
      entry.crc32 = filestat.m_crc32;
      entries.push_back(entry);
    }

    if (!mz_zip_reader_end(&ziparchive))
    {
      throw std::runtime_error("Could not close zip archive file " + filename);
    }
  }

  std::vector<EntryInfo> entries;
};

/*
* Create a sample directory hierarchy in tempdir
* to perform testing of ResourceCompiler
*/
static void createDirHierarchy()
{
  /*
   * We create the following directory hierarchy
   * current_dir/
   *    |____ manifest.json
   *    |____ sample.dll
   *    |____ resource1/
   *    |         |______ resource1.txt
   *    |____ resource2/
   *              |______ resource2.txt
   */

  std::ofstream manifest("manifest.json");
  if (!manifest.is_open())
  {
    std::runtime_error("Couldn't open 'manifest.json'");
  }
  manifest << manifest_json << std::endl;
  manifest.close();

  testing::MakeDirectory("resource1");
  testing::MakeDirectory("resource2");

  std::string rc1str = "resource1";
  rc1str += testing::DIR_SEP + "resource1.txt";
  std::string rc2str = "resource2";
  rc2str += testing::DIR_SEP + "resource2.txt";
  std::ofstream rc1file(rc1str.c_str());
  std::ofstream rc2file(rc2str.c_str());
  if (!rc1file.is_open())
  {
    std::runtime_error("Couldn't open 'resource1/resource1.txt'");
  }
  if (!rc2file.is_open())
  {
    std::runtime_error("Couldn't open 'resource2/resource2.txt'");
  }

  rc1file << resource1_txt << std::endl;
  rc1file.close();
  rc2file << resource2_txt << std::endl;
  rc2file.close();

  char data[5] = { 0, 2, 3, 4, 5 };
  std::ofstream dll("sample.dll");
  if (!dll.is_open())
  {
    std::runtime_error("Couldn't open 'sample.dll'");
  }
  dll.write(data, sizeof(data)); //binary write
  dll.close();
}

/* 
* Removes all the stray files and directories. brings back to clean slate
*/
static void removeDirHierarchy()
{
  // remove files
  remove("manifest.json");
  std::string rc1str = "resource1";
  rc1str += testing::DIR_SEP + "resource1.txt";
  std::string rc2str = "resource2";
  rc2str += testing::DIR_SEP + "resource2.txt";
  remove(rc1str.c_str());
  remove(rc2str.c_str());
  // remove dirs
  RemoveDirectory("resource1");
  RemoveDirectory("resource2");
}

/*
* 1. Use resource compiler to create Example.zip with just manifest.json
*/
static void testrc1(const std::string& rcbinpath)
{
  std::string cmd(rcbinpath);
  cmd += " --bundle-name mybundle ";
  cmd += " --out-file Example.zip ";
  cmd += " --manifest-add manifest.json";
  std::system(cmd.c_str());

  ZipFile zip("Example.zip");
  US_TEST_CONDITION(zip.entries.size() == 2, "Check number of entries of zip.")
  US_TEST_CONDITION(zip.entries[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip.entries[1].name == "mybundle/", "Check name of entry 1.")
}

/*
* 2. Use resource compiler to create Example.zip with manifest.json and
* one --res-add option
*/
static void testrc2(const std::string& rcbinpath)
{
  std::string cmd(rcbinpath);
  cmd += " --bundle-name mybundle ";
  cmd += " --out-file Example2.zip ";
  cmd += " --manifest-add manifest.json ";
  cmd += " --res-add resource1/resource1.txt";
  std::system(cmd.c_str());

  ZipFile zip("Example2.zip");
  US_TEST_CONDITION(zip.entries.size() == 4, "Check number of entries of zip.")
  US_TEST_CONDITION(zip.entries[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip.entries[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip.entries[2].name == "mybundle/resource1/resource1.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip.entries[3].name == "mybundle/resource1/", "Check name of entry 3.")
}

/*
* 3. Use resource compiler to create tomerge.zip with only --res-add option
*/
static void testrc3(const std::string& rcbinpath)
{
  std::string cmd(rcbinpath);
  cmd += " --bundle-name mybundle ";
  cmd += " --out-file tomerge.zip ";
  cmd += " --res-add resource2/resource2.txt";
  std::system(cmd.c_str());

  ZipFile zip("tomerge.zip");
  US_TEST_CONDITION(zip.entries.size() == 3, "Check number of entries of zip.")
  US_TEST_CONDITION(zip.entries[0].name == "mybundle/resource2/resource2.txt", "Check name of entry 0.")
  US_TEST_CONDITION(zip.entries[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip.entries[2].name == "mybundle/resource2/", "Check name of entry 2.")
}

/*
* 4. Use resource compiler to create Example4.zip
* add a manifest, add resource1/resource1.txt using --res-add
* and merge tomerge.zip into Example4.zip using --zip-add
*/
static void testrc4(const std::string& rcbinpath)
{
  std::string cmd(rcbinpath);
  cmd += " --bundle-name mybundle ";
  cmd += " --manifest-add manifest.json ";
  cmd += " --out-file Example4.zip ";
  cmd += " --res-add resource1/resource1.txt ";
  cmd += " --zip-add tomerge.zip ";
  std::system(cmd.c_str());

  ZipFile zip("Example4.zip");
  US_TEST_CONDITION(zip.entries.size() == 6, "Check number of entries of zip.")
  US_TEST_CONDITION(zip.entries[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip.entries[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip.entries[2].name == "mybundle/resource1/resource1.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip.entries[3].name == "mybundle/resource1/", "Check name of entry 3.")
  US_TEST_CONDITION(zip.entries[4].name == "mybundle/resource2/resource2.txt", "Check name of entry 4.")
  US_TEST_CONDITION(zip.entries[5].name == "mybundle/resource2/", "Check name of entry 5.")
}

/*
* 5. Use resource compiler to append a zip-file (using --zip-add)
* to the bundle sample.dll using the -b option and also add a manifest
* while doing so.
*/
static void testrc5(const std::string& rcbinpath)
{
  std::string cmd(rcbinpath);
  cmd += " --bundle-file sample.dll ";
  cmd += " --bundle-name mybundle ";
  cmd += " --manifest-add manifest.json ";
  cmd += " --zip-add tomerge.zip ";
  std::system(cmd.c_str());

  ZipFile zip("sample.dll");
  US_TEST_CONDITION(zip.entries.size() == 4, "Check number of entries of zip.")
  US_TEST_CONDITION(zip.entries[0].name == "mybundle/manifest.json", "Check name of entry 0.")
  US_TEST_CONDITION(zip.entries[1].name == "mybundle/", "Check name of entry 1.")
  US_TEST_CONDITION(zip.entries[2].name == "mybundle/resource2/resource2.txt", "Check name of entry 2.")
  US_TEST_CONDITION(zip.entries[3].name == "mybundle/resource2/", "Check name of entry 3.")
}

static void removeOutFiles()
{
  remove("Example.zip");
  remove("Example2.zip");
  remove("Example4.zip");
  remove("tomerge.zip");
  remove("sample.dll");
}

int ResourceCompilerTest(int argc, char* argv[])
{
  US_TEST_BEGIN("ResourceCompilerTest");
  auto const binpath = testing::GetExecutablePath();

  auto bindir = testing::getUptoLastDir(binpath);
  auto rcbinpath = bindir + "usResourceCompiler.exe";
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

  testing::ChangeDirectory(tempdir);

  createDirHierarchy();

  testrc1(rcbinpath);

  testrc2(rcbinpath);

  testrc3(rcbinpath);

  testrc4(rcbinpath);

  testrc5(rcbinpath);

  removeDirHierarchy();
  removeOutFiles();

  US_TEST_END()
}
