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

#include "TestUtils.h"
#include "TestingConfig.h"
#include "ZipFile.h"

#include "gtest/gtest.h"
#include "json/json.h"

#include <array>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace cppmicroservices;
using namespace cppmicroservices::util;
using namespace cppmicroservices::testing;
namespace
{

    // MakeUniqueTempDirectory provides a temp dir with unique alphanumeric name
    // uniqueTempdir is global as it needs to be a constant for all the tests
    TempDir uniqueTempdir = MakeUniqueTempDirectory();

    // The value returned by the resource compiler when manifest validation failed
    static int const BUNDLE_MANIFEST_VALIDATION_ERROR_CODE(2);

    class ResourceCompilerTest : public ::testing::Test
    {
      public:
        ResourceCompilerTest() = default;
        ~ResourceCompilerTest() override = default;

        void
        SetUp() override
        {
            rcbinpath = RCC_PATH;
            /*
             * If ResourceCompiler executable is not found, we can't run the tests, we
             * mark it as a failure and exit
             */
            std::ifstream binf(rcbinpath.c_str());
            ASSERT_TRUE(binf.good()) << "Cannot find usResourceCompiler executable:" << rcbinpath;

            tempdir = uniqueTempdir.Path + DIR_SEP;
        }

      protected:
        std::string rcbinpath;
        std::string tempdir;
    };

    // A zip file containing only a manifest.
    class ManifestZipFile
    {
      public:
        // zip_file_name - zip file path
        // manifest_json - contents of the manifest.json file
        // bundle_name - name of the bundle
        ManifestZipFile(std::string const& zip_file_name,
                        std::string const& manifest_json,
                        std::string const& bundle_name)
        {
            std::string archiveEntry(bundle_name + "/manifest.json");
            mz_zip_archive zip;
            memset(&zip, 0, sizeof(mz_zip_archive));

            mz_zip_writer_init_file(&zip, zip_file_name.c_str(), 0);
            mz_zip_writer_add_mem(&zip,
                                  archiveEntry.c_str(),
                                  manifest_json.c_str(),
                                  manifest_json.size() * sizeof(std::string::value_type),
                                  MZ_DEFAULT_COMPRESSION);
            mz_zip_writer_finalize_archive(&zip);
            mz_zip_writer_end(&zip);
        }

        ManifestZipFile(ManifestZipFile const&) = delete;
        void operator=(ManifestZipFile const&) = delete;
        ManifestZipFile(ManifestZipFile const&&) = delete;
        ManifestZipFile& operator=(ManifestZipFile const&&) = delete;

        ~ManifestZipFile() {}
    };

    /*
     * Transform the path specified in "path", which may contain a combination of spaces
     * and parenthesis, to a path with the spaces and parenthesis escaped with "\".
     * If this isn't escaped, test invocation (std::system) with this path results
     * in an error that the given file is not found.
     *
     * E.g. "/tmp/path (space)/rc" will result in a raw "/tmp/path\ \(space\)/rc"
     */
    void
    escapePath(std::string& path)
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
     * Create a sample directory hierarchy in tempdir
     * to perform testing of ResourceCompiler
     */
    void
    createDirHierarchy(std::string const& tempdir, std::string const& manifest_json)
    {
        /*
         * We create the following directory hierarchy
         * current_dir/
         *    |____ manifest.json
         *    |____ sample.dll
         *    |____ sample1.dll
         *    |____ ｆｏｏｂａｒ
         *              |______ myArchive.zip
         *    |____ resource1/
         *    |         |______ resource1.txt
         *    |____ resource2/
         *              |______ resource2.txt
         */

        const std::string resource1_txt = "A sample resource to embed\n"
                                          "Inside the text file resource1.txt";

        const std::string resource2_txt = "A sample resource to embed\n"
                                          "Inside the text file resource2.txt";

        std::string manifest_fpath(tempdir + "manifest.json");
        std::ofstream manifest(manifest_fpath);
        ASSERT_TRUE(manifest.is_open()) << "Couldn't open " << manifest_fpath;
        manifest << manifest_json << std::endl;
        manifest.close();

        std::string unicodePath(tempdir + u8"ｆｏｏｂａｒ");
        MakePath(unicodePath);
        std::ifstream manifest_src(manifest_fpath, std::ios::binary);
        std::ofstream manifest_dst(unicodePath + "/manifest.json", std::ios::binary);
        manifest_dst << manifest_src.rdbuf();
        manifest_src.close();
        manifest_dst.close();

        std::string rc1dir_path(tempdir + "resource1");
        std::string rc2dir_path(tempdir + "resource2");
        MakePath(rc1dir_path);
        MakePath(rc2dir_path);
        std::string rc1file_path(rc1dir_path + DIR_SEP + "resource1.txt");
        std::string rc2file_path(rc2dir_path + DIR_SEP + "resource2.txt");
        std::ofstream rc1file(rc1file_path.c_str());
        std::ofstream rc2file(rc2file_path.c_str());
        ASSERT_TRUE(rc1file.is_open()) << "Couldn't open " << rc1file_path;
        ASSERT_TRUE(rc2file.is_open()) << "Couldn't open " << rc2file_path;

        rc1file << resource1_txt << std::endl;
        rc1file.close();
        rc2file << resource2_txt << std::endl;
        rc2file.close();

        // Create 2 binary files filled with random numbers
        // to test bundle-file functionality.
        auto create_mock_dll = [&tempdir](std::string const& dllname, std::array<char, 5> const& dat)
        {
            std::string dll_path(tempdir + dllname);
            std::ofstream dll(dll_path.c_str());
            ASSERT_TRUE(dll.is_open()) << "Couldn't open " << dll_path;

            dll.write(dat.data(), dat.size()); // binary write
            dll.close();
        };
        std::array<char, 5> data1 = {
            {2, 4, 6, 8, 10}
        };
        std::array<char, 5> data2 = {
            {1, 2, 3, 4, 5}
        };
        create_mock_dll("sample.dll", data1);
        create_mock_dll("sample1.dll", data2);
    }

    /*
     * Execute a process with the given command line
     * @param executable absolute path to the executable to run along with
     *  and any command line parameters.
     * @return the executable's return code.
     */
    int
    runExecutable(std::string const& executable)
    {
        // WEXITSTATUS is only available on POSIX. Wrap std::system into a function
        // call so that there is consistent and uniform return codes on all platforms.
#if defined US_PLATFORM_WINDOWS
#    define WEXITSTATUS
#endif

        int ret = std::system(executable.c_str());

        // WEXITSTATUS uses an old c-sytle cast
        // clang-format off
  US_GCC_PUSH_DISABLE_WARNING(all)
        // clang-format on
        return WEXITSTATUS(ret);
        US_GCC_POP_WARNING
    }

    /*
     * In a vector of strings "entryNames", test if the string "name" exists
     */
    void
    testExists(std::vector<std::string> const& entryNames, std::string const& name)
    {
        bool exists = std::find(entryNames.begin(), entryNames.end(), name) != entryNames.end();
        ASSERT_TRUE(exists) << "Check existence of " << name;
    }

    // Create a manifest.json file
    void
    createManifestFile(const std::string tempdir,
                       const std::string manifest_json,
                       const std::string manifest_json_file = "manifest.json")
    {
        std::string manifest_fpath(tempdir + manifest_json_file);
        std::ofstream manifest(manifest_fpath);
        EXPECT_TRUE(manifest.is_open()) << "Couldn't open " << manifest_fpath;

        manifest << manifest_json << std::endl;
        manifest.close();
    }

    // Create a fake shared library which will be used to test embedding invalid manifest data
    // using usResourceCompiler.
    void
    createDummyBundle(const std::string tempdir, const std::string bundle_name)
    {
        // Create a binary file filled with random numbers
        // to test usResourceCompiler functionality.
        auto create_mock_dll = [&tempdir](std::string const& dllname, std::array<char, 5> const& dat)
        {
            std::string dll_path(tempdir + dllname);
            std::ofstream dll(dll_path.c_str());
            EXPECT_TRUE(dll.is_open()) << "Couldn't open " << dll_path;
            dll.write(dat.data(), dat.size()); // binary write
            dll.close();
        };
        std::array<char, 5> data1 = {
            {2, 4, 6, 8, 10}
        };
        create_mock_dll(bundle_name, data1);
    }

    // A helper function designed to validate whether embedding an invalid manifest
    // to a fake shared library worked or not.
    // returns true if there is a zip file contaning at least one entry in it.
    // returns false otherwise.
    bool
    containsBundleZipFile(const std::string bundle_file_path)
    {
        try
        {
            ZipFile bundle(bundle_file_path);
            return (bundle.size() > 0);
        }
        catch (...)
        {
            return false;
        }
        return false;
    }

    // return the contents of the manifest file for the given bundle_name
    std::string
    getManifestContent(std::string const& zipfile, std::string bundle_name)
    {
        mz_zip_archive zipArchive;
        memset(&zipArchive, 0, sizeof(mz_zip_archive));

        EXPECT_TRUE(mz_zip_reader_init_file(&zipArchive, zipfile.c_str(), 0))
            << "Could not initialize zip archive " << zipfile;

        std::string manifestArchiveEntry(bundle_name + "/manifest.json");
        char archiveEntryContents[MZ_ZIP_MAX_IO_BUF_SIZE];
        memset(archiveEntryContents, 0, MZ_ZIP_MAX_IO_BUF_SIZE);
        if (MZ_TRUE
            == mz_zip_reader_extract_file_to_mem(&zipArchive,
                                                 manifestArchiveEntry.c_str(),
                                                 archiveEntryContents,
                                                 MZ_ZIP_MAX_IO_BUF_SIZE,
                                                 0))
        {
            mz_zip_reader_end(&zipArchive);
            return std::string(archiveEntryContents);
        }

        mz_zip_reader_end(&zipArchive);
        return std::string();
    }

    /*
     * @brief remove any line feed and new line characters.
     * @param[in,out] str string to be modified
     */
    void
    removeLineEndings(std::string& str)
    {
        str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    }
} // namespace

TEST_F(ResourceCompilerTest, testEscapePath)
{
    // Test escapePath function
    std::string path1("/tmp/path (space)/rc");
    escapePath(path1);
    // Test escapePath #1
    ASSERT_EQ(path1, "/tmp/path\\ \\(space\\)/rc");

    std::string path2("/tmp/foo/bar");
    escapePath(path2);
    // Test escapePath #2
    ASSERT_EQ(path2, "/tmp/foo/bar");

    std::string path3("/home/runner/CppMicroServices/us builds (Unix "
                      "make)/bin/usResourceCompiler");
    escapePath(path3);
    // Test escapePath #3
    ASSERT_EQ(path3,
              "/home/runner/CppMicroServices/us\\ builds\\ "
              "\\(Unix\\ make\\)/bin/usResourceCompiler");

#ifndef US_PLATFORM_WINDOWS
    escapePath(tempdir);
    escapePath(rcbinpath);
#endif
}

TEST_F(ResourceCompilerTest, testManifestAdd)
{
    const std::string manifest_json = R"({
      "bundle.symbolic_name" : "mybundle",
      "bundle.version" : "0.1.0",
      "bundle.activator" : true
    })";
    createDirHierarchy(tempdir, manifest_json);

    std::ostringstream badcmd;
    badcmd << rcbinpath
        << " --bundle-name mismatched_bundle_name"
        << " --out-file \"" << tempdir << "Example.zip\""
        << " --manifest-add \"" << tempdir << "manifest.json\"";
    // Test that invoking command with --bundle-name different from that in the manifest fails.
    ASSERT_NE(EXIT_SUCCESS, runExecutable(badcmd.str()));

    std::ostringstream cmd;
    cmd << rcbinpath
        << " --bundle-name mybundle"
        << " --out-file \"" << tempdir << "Example.zip\""
        << " --manifest-add \"" << tempdir << "manifest.json\"";
    // Test that Cmdline invocation in testManifestAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ZipFile zip(tempdir + DIR_SEP + "Example.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 2);

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
TEST_F(ResourceCompilerTest, testManifestResAdd)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --out-file Example2.zip ";
    cmd << " --manifest-add manifest.json ";
    cmd << " --res-add resource1/resource1.txt";

    auto cwdir = GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Test that Cmdline invocation in testManifestResAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
    ChangeDirectory(cwdir);

    ZipFile zip(tempdir + "Example2.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 4);

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
TEST_F(ResourceCompilerTest, testResAdd)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --out-file tomerge.zip ";
    cmd << " --res-add resource2/resource2.txt";

    auto cwdir = GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Test that Cmdline invocation in testResAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
    ChangeDirectory(cwdir);

    ZipFile zip(tempdir + "tomerge.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 3);

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
TEST_F(ResourceCompilerTest, testZipAdd)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --manifest-add manifest.json ";
    cmd << " --out-file Example4.zip ";
    cmd << " --res-add resource1/resource1.txt ";
    cmd << " --zip-add tomerge.zip";

    auto cwdir = GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Test that Cmdline invocation in testZipAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
    ChangeDirectory(cwdir);

    ZipFile zip(tempdir + "Example4.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 6);

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
TEST_F(ResourceCompilerTest, testZipAddBundle)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --bundle-file \"" << tempdir << "sample.dll\" ";
    cmd << " --manifest-add \"" << tempdir << "manifest.json\" ";
    cmd << " --zip-add \"" << tempdir << "tomerge.zip\"";

    // Test that Cmdline invocation in testResAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ZipFile zip(tempdir + "sample.dll");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 4);

    auto entryNames = zip.getNames();
    testExists(entryNames, "mybundle/manifest.json");
    testExists(entryNames, "mybundle/");
    testExists(entryNames, "mybundle/resource2/resource2.txt");
    testExists(entryNames, "mybundle/resource2/");
}

/*
 * Add two zip-add arguments to a bundle-file.
 */
TEST_F(ResourceCompilerTest, testZipAddTwice)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file \"" << tempdir << "sample1.dll\" ";
    cmd << " --zip-add \"" << tempdir << "tomerge.zip\" ";
    cmd << " --zip-add \"" << tempdir << "Example2.zip\"";

    // Test that Cmdline invocation in testZipAddTwice returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ZipFile zip(tempdir + "sample1.dll");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 6);

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
TEST_F(ResourceCompilerTest, testBundleManifestZipAdd)
{
    const std::string manifest_json = R"({
      "bundle.symbolic_name" : "anotherbundle",
      "bundle.version" : "0.1.0",
      "bundle.activator" : true
    })";
    createManifestFile(tempdir, manifest_json, "manifest2.json");
    
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --manifest-add \"" << tempdir << "manifest2.json\" ";
    cmd << " --bundle-file \"" << tempdir << "sample1.dll\" ";
    cmd << " --zip-add \"" << tempdir << "tomerge.zip\" ";
    cmd << " --zip-add \"" << tempdir << "Example2.zip\"";

    // Test that Cmdline invocation in testBundleManifestZipAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ZipFile zip(tempdir + "sample1.dll");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 8);

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
 * Use resource compiler to create zip files with various compression levels
 *
 * Working directory is changed temporarily to tempdir because of --res-add option
 */
TEST_F(ResourceCompilerTest, testCompressionLevel)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --manifest-add manifest.json ";
    cmd << " --out-file ExampleCompressionLevel0.zip ";
    cmd << " --res-add resource1/resource1.txt ";
    cmd << " --zip-add tomerge.zip";
    cmd << " --compression-level 0";

    auto cwdir = util::GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Test that --compression-level = 0 successfully creates a zip file.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
    ChangeDirectory(cwdir);

    ZipFile zip(tempdir + "ExampleCompressionLevel0.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 6);

    auto entryNames = zip.getNames();
    testExists(entryNames, "mybundle/manifest.json");
    testExists(entryNames, "mybundle/");
    testExists(entryNames, "mybundle/resource1/resource1.txt");
    testExists(entryNames, "mybundle/resource1/");
    testExists(entryNames, "mybundle/resource2/resource2.txt");
    testExists(entryNames, "mybundle/resource2/");

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --manifest-add manifest.json ";
    cmd << " --out-file ExampleCompressionLevel3.zip ";
    cmd << " --res-add resource1/resource1.txt ";
    cmd << " --zip-add tomerge.zip";
    cmd << " --compression-level 3";

    cwdir = util::GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Test that --compression-level = 3 successfully creates a zip file.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
    ChangeDirectory(cwdir);

    zip = ZipFile(tempdir + "ExampleCompressionLevel3.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 6);

    entryNames = zip.getNames();
    testExists(entryNames, "mybundle/manifest.json");
    testExists(entryNames, "mybundle/");
    testExists(entryNames, "mybundle/resource1/resource1.txt");
    testExists(entryNames, "mybundle/resource1/");
    testExists(entryNames, "mybundle/resource2/resource2.txt");
    testExists(entryNames, "mybundle/resource2/");
}

/*
 * Add the same manifest contents multiples times through --manifest-add
 * The intended behavior is that any subsequent duplicate manifest file is ignored
 */
TEST_F(ResourceCompilerTest, testDuplicateManifestFileAdd)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --manifest-add \"" << tempdir << "manifest.json\" ";
    cmd << " --manifest-add \"" << tempdir << "manifest.json\" ";
    cmd << " --out-file \"" << tempdir << "testDuplicateManifestFileAdd.zip\" ";

    // Test that Cmdline invocation in testDuplicateManifestFileAdd returns 0
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ZipFile zip(tempdir + "testDuplicateManifestFileAdd.zip");
    // Check number of entries of zip.
    ASSERT_EQ(zip.size(), 2);
}

/*
 * --help returns exit code 0
 */
TEST_F(ResourceCompilerTest, testHelpReturnsZero)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --help";

    // validate that help option returns zero
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
}

/*
 * Test the failure modes of ResourceCompiler command
 */
TEST_F(ResourceCompilerTest, testFailureModes)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name foo";
    cmd << " --manifest-add ";
    cmd << " --bundle-file test2.dll";
    // Test Failure mode: Empty --manifest-add option
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name foo";
    cmd << " --manifest-add file_does_not_exist.json";
    cmd << " --bundle-file test2.dll";
    // Test Failure mode: Manifest file does not exist
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-file test1.dll ";
    cmd << " --bundle-file test2.dll";
    // test Failure mode: Multiple bundle-file args
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --out-file test1.zip ";
    cmd << " --out-file test2.zip";
    // Test Failure mode: Multiple out-file args
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name foo ";
    cmd << " --bundle-name bar ";
    cmd << " --bundle-file bundlefile";
    // Test Failure mode: Multiple bundle-name args
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --manifest-add manifest.json";
    cmd << " --bundle-name foobundle";
    // Test Failure mode: --bundle-file or --out-file required
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --manifest-add manifest.json";
    // Test Failure mode: --bundle-name required
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --res-add manifest.json";
    // Test Failure mode: --bundle-name required
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --manifest-add manifest.json";
    cmd << " --bundle-name foo ";
    cmd << " --compression-level 11";
    // Test Failure mode: invalid --compression-level argument (11)
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --manifest-add manifest.json";
    cmd << " --bundle-name foo ";
    cmd << " --compression-level -1";
    // Test Failure mode: invalid --compression-level argument (-1)
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle ";
    cmd << " --bundle-file \"" << tempdir << "sample1.dll\" ";
    cmd << " --zip-add \"" << tempdir << "tomerge.zip\" ";
    cmd << " --zip-add \"" << tempdir << "Example2.zip\"";
    // test --bundle-name arg without either --manifest-add or --res-add is just a warning
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    // Example.zip already contains mybundle/manifest.json
    // Should get an error when we are trying to manifest-add
    // mybundle/manifest.json
    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "mybundle";
    cmd << " --out-file \"" << tempdir << "Example7.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest.json\"";
    cmd << " --zip-add \"" << tempdir << "Example.zip\"";
    // Test Failure mode: duplicate manifest.json
    ASSERT_EQ(EXIT_FAILURE, runExecutable(cmd.str()));
}

// Test the multiple ways in which a manifest can be added...
// --manifest-add to a user defined zip file (using --out-file)
// --manifest-add to a shared library (using --bundle-file)
// --res-add to a user defined zip file (using --out-file)
// --res-add to a shared library (using --bundle-file)
// --zip-add to merge with another zip file (using --out-file)
// --zip-add to append to a bundle (using --bundle-file)
//
// in all cases, the invalid manifest.json should never be added
// to the zip or shared library.
TEST_F(ResourceCompilerTest, testManifestAddWithInvalidJSON)
{
    std::string invalidSyntax(R"(
		{
		 "bundle.name": "foo",
		 "bundle.version" : "1.0.2"
		 "bundle.description" : "This bundle is broken!",
		 "authors" : ["John Doe", "Douglas Reynolds", "Daniel Cannady"],
		 "rating" : 5
		} 
    )");

    createManifestFile(tempdir, invalidSyntax);

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "mybundle";
    cmd << " --out-file \"" << tempdir << "invalid_syntax.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest.json\"";
    // Fail to embed manifest containing JSON syntax errors.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd.str()));

    ZipFile invalid_syntax_zip(tempdir + "invalid_syntax.zip");
    // Test that the invalid manifest.json file was not added
    ASSERT_EQ(0, invalid_syntax_zip.size());

    std::string invalid_syntax_bundle_name("invalid_syntax.bundle");
    createDummyBundle(tempdir, invalid_syntax_bundle_name);

    std::ostringstream cmd2;
    cmd2 << rcbinpath;
    cmd2 << " --bundle-name "
         << "mybundle";
    cmd2 << " --bundle-file \"" << tempdir << invalid_syntax_bundle_name << "\"";
    cmd2 << " --manifest-add \"" << tempdir << "manifest.json\"";
    // Fail to embed manifest containing JSON syntax errors
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd2.str()));
    // Test that the invalid manifest.json file was not added
    ASSERT_FALSE(containsBundleZipFile(tempdir + invalid_syntax_bundle_name));

    std::ostringstream cmd3;
    cmd3 << rcbinpath;
    cmd3 << " --bundle-name mybundle ";
    cmd3 << " --out-file invalid_syntax_res_add.zip ";
    cmd3 << " --res-add manifest.json";

    auto origdir = util::GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Fail to embed manifest containing JSON syntax errors.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd3.str()));
    ChangeDirectory(origdir);

    ZipFile invalid_syntax_res_add_zip(tempdir + "invalid_syntax_res_add.zip");
    // Test that the invalid manifest.json file was not added
    ASSERT_EQ(0, invalid_syntax_res_add_zip.size());

    std::string invalid_syntax_res_add_bundle_name("invalid_syntax_res_add.bundle");

    std::ostringstream cmd4;
    cmd4 << rcbinpath;
    cmd4 << " --bundle-name mybundle ";
    cmd4 << " --bundle-file " << invalid_syntax_res_add_bundle_name;
    cmd4 << " --res-add manifest.json";

    ChangeDirectory(tempdir);
    // Fail to embed manifest containing JSON syntax errors.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd4.str()));
    ChangeDirectory(origdir);
    // Test that the invalid manifest.json file was not added
    ASSERT_FALSE(containsBundleZipFile(tempdir + invalid_syntax_res_add_bundle_name));
}

TEST_F(ResourceCompilerTest, testUnicodeBundleFile)
{
  
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file \"" << tempdir << u8"ｆｏｏｂａｒ/myArchive00.zip\"";

    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
}

TEST_F(ResourceCompilerTest, testUnicodeBundleName)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file \"" << tempdir << u8"ｆｏｏｂａｒ/myArchive01.zip\"";
    cmd << " --bundle-name ｆｏｏｂａｒ";

    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
}

TEST_F(ResourceCompilerTest, testUnicodeOutFile)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file \"" << tempdir << u8"ｆｏｏｂａｒ/myArchive02.zip\"";
    cmd << " --out-file \"" << tempdir << u8"ｆｏｏｂａｒ123\"";

    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
}

TEST_F(ResourceCompilerTest, testUnicodeResAdd)
{
    auto origdir = util::GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file " << u8"ｆｏｏｂａｒ/myArchive03.zip";
    cmd << " --out-file unicodeResAdd00.zip";
    cmd << " --res-add " << u8"ｆｏｏｂａｒ/manifest.json";
    cmd << " --bundle-name myunicodebundle00";
    cmd << " -V";

    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ChangeDirectory(origdir);
}

TEST_F(ResourceCompilerTest, testUnicodeZipAdd)
{
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file \"" << tempdir << u8"ｆｏｏｂａｒ/myArchive04.zip\"";
    cmd << " --out-file \"" << tempdir << "unicodeResAdd01.zip\"";
    cmd << " --zip-add \"" << tempdir << "tomerge.zip\"";
    cmd << " --bundle-name myunicodebundle01";

    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
}

TEST_F(ResourceCompilerTest, testUnicodeManifestAdd)
{
    const std::string manifest_json = u8R"({
      "bundle.symbolic_name" : "myunicodebundle02-V",
      "bundle.version" : "0.1.0",
      "bundle.activator" : true
    })";
    createManifestFile(tempdir, manifest_json, "manifest3.json");
    
    auto origdir = util::GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-file " << u8"ｆｏｏｂａｒ/myArchive05.zip";
    cmd << " --out-file unicodeResAdd02.zip";
    cmd << " --manifest-add " << u8"manifest3.json";
    cmd << " --bundle-name myunicodebundle02";
    cmd << "-V";

    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ChangeDirectory(origdir);
}

// In jsoncpp 0.10.6 not allowing comments does NOT result in a parse failure for a JSON file with comments.
// Instead, assuming the JSON is valid, parsing returns JSON stripped of the comments.
// This test will only makes sure that JSON with comments can be added successfully.
// It shouldn't be necessary to check that JSON comments are actually stripped from the output json as
// that should be the responsibility of jsoncpp's tests and we will rely on that.
// There is an issue logged for this behavior in jsoncpp (https://github.com/open-source-parsers/jsoncpp/issues/690)
TEST_F(ResourceCompilerTest, testManifestAddWithJSONComments)
{
    std::string jsonCommentSyntax(R"(
		{ /* no, no, no. */
		 "bundle.name": "foo",
		 "bundle.symbolic_name": "mybundle",
		 "bundle.version" : "1.0.2",
		 "bundle.description" : "This bundle shouldn't have comments!",
		 "authors" : ["John Doe", "Douglas Reynolds", "Daniel Cannady"],
		 "rating" : 5 // no comments allowed
		} 
    )");

    createManifestFile(tempdir, jsonCommentSyntax, "manifest4.json");

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name mybundle";
    cmd << " --out-file \"" << tempdir << "json_comment_syntax.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest4.json\"";
    // Test embedding a manifest containing JSON comments.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));
}

TEST_F(ResourceCompilerTest, testManifestAddWithDuplicateKeys)
{
    std::string dupKeys(R"(
		{
		 "bundle.name": "foo",
		 "bundle.version" : "1.0.2",
		 "bundle.description" : "This bundle is broken!",
		 "authors" : ["John Doe", "Douglas Reynolds", "Daniel Cannady"],
		 "bundle.name" : "foobar",
		 "rating" : 5
		}
    )");

    createManifestFile(tempdir, dupKeys);

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "mybundle";
    cmd << " --out-file \"" << tempdir << "duplicate_keys.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest.json\"";
    // Fail to embed manifest containing duplicate JSON key names.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd.str()));

    ZipFile duplicate_keys_zip(tempdir + "duplicate_keys.zip");
    // Test that the invalid manifest.json file was not added
    ASSERT_EQ(0, duplicate_keys_zip.size());

    std::string duplicate_keys_bundle_file("duplicate_keys.bundle");
    std::ostringstream cmd2;
    cmd2 << rcbinpath;
    cmd2 << " --bundle-name "
         << "mybundle";
    cmd2 << " --bundle-file \"" << tempdir << duplicate_keys_bundle_file << "\"";
    cmd2 << " --manifest-add \"" << tempdir << "manifest.json\"";
    // Fail to embed manifest containing duplicate JSON key names.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd2.str()));

    // Test that the invalid manifest.json file was not added
    ASSERT_FALSE(containsBundleZipFile(tempdir + duplicate_keys_bundle_file));

    std::ostringstream cmd3;
    cmd3 << rcbinpath;
    cmd3 << " --bundle-name mybundle ";
    cmd3 << " --out-file duplicate_keys_res_add.zip ";
    cmd3 << " --res-add manifest.json";

    auto origdir = util::GetCurrentWorkingDirectory();
    ChangeDirectory(tempdir);
    // Fail to embed manifest containing duplicate JSON key names.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd3.str()));
    ChangeDirectory(origdir);

    ZipFile duplicate_keys_res_add_zip(tempdir + "duplicate_keys_res_add.zip");
    // Test that the invalid manifest.json file was not added
    ASSERT_EQ(0, duplicate_keys_res_add_zip.size());

    std::string duplicate_keys_res_add_bundle_file("duplicate_keys_res_add.bundle");
    std::ostringstream cmd4;
    cmd4 << rcbinpath;
    cmd4 << " --bundle-name mybundle ";
    cmd4 << " --bundle-file " << duplicate_keys_res_add_bundle_file;
    cmd4 << " --res-add manifest.json";

    ChangeDirectory(tempdir);
    // Fail to embed manifest containing duplicate JSON key names.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd4.str()));
    ChangeDirectory(origdir);
    // Test that the invalid manifest.json file was not added
    ASSERT_FALSE(containsBundleZipFile(tempdir + duplicate_keys_res_add_bundle_file));
}

// Use case: A zip file created using an older usResourceCompiler (which doesn't validate manifest.json)
// and containing an invalid manifest.json is appended to a bundle.
TEST_F(ResourceCompilerTest, testAppendZipWithInvalidManifest)
{
    const std::string manifest_json = R"({
     "bundle.symbolic_name" : "main",
     "bundle.version" : "0.1.0",
     "bundle.activator" : true
    })";

    createDirHierarchy(tempdir, manifest_json);

    std::ostringstream cmdCreateBundle;
    cmdCreateBundle << rcbinpath;
    cmdCreateBundle << " --bundle-name main ";
    cmdCreateBundle << " --bundle-file \"" << tempdir << "sample.dll\" ";
    cmdCreateBundle << " --manifest-add \"" << tempdir << "manifest.json\" ";
    // Test that the manifest.json file was embedded correctly.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmdCreateBundle.str()));

    std::string invalidSyntax(R"(
		{
         "bundle.name": "foo",
         "bundle.version" : "1.0.2"
         "bundle.description" : "This bundle is broken!",
         "authors" : ["John Doe", "Douglas Reynolds", "Daniel Cannady"],
         "rating" : 5
		} 
    )");

    ManifestZipFile badZip(tempdir + "append_invalid_manifest_zip.zip", invalidSyntax, "invalid");
    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "invalid";
    cmd << " --bundle-file \"" << tempdir << "sample.dll\"";
    cmd << " --zip-add \"" << tempdir << "append_invalid_manifest_zip.zip\"";
    // Fail to append a zip file containing an invalid manifest.json
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd.str()));

    ZipFile append_invalid_manifest_zip(tempdir + "sample.dll");

    // Test that the invalid manifest.json file was not appended to the bundle.
    ASSERT_EQ(2, append_invalid_manifest_zip.size());
    // Test that only the valid manifest.json exists.
    ASSERT_EQ("main/manifest.json", append_invalid_manifest_zip[0].name);
}

// Use case: A zip file created using an older usResourceCompiler (which doesn't validate manifest.json)
// and containing an invalid manifest.json is merged with a valid zip file.
TEST_F(ResourceCompilerTest, testZipMergeWithInvalidManifest)
{
    std::string const manifest_json = R"({
    "bundle.symbolic_name" : "main",
    "bundle.version" : "0.1.0",
    "bundle.activator" : true
    })";

    ManifestZipFile validZip(tempdir + "merged_zip.zip", manifest_json, "main");

    std::string invalidSyntax(R"(
		{
		 "bundle.name": "foo",
		 "bundle.version" : "1.0.2"
		 "bundle.description" : "This bundle is broken!",
		 "authors" : ["John Doe", "Douglas Reynolds", "Daniel Cannady"],
		 "rating" : 5
		} 
    )");

    ManifestZipFile badZip(tempdir + "x_invalid_manifest.zip", invalidSyntax, "invalid");

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "main";
    cmd << " --out-file \"" << tempdir << "new_merged_zip.zip\"";
    cmd << " --zip-add \"" << tempdir << "merged_zip.zip\"";
    cmd << " --zip-add \"" << tempdir << "x_invalid_manifest.zip\"";
    // Fail to merge a zip file containing an invalid manifest.json
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd.str()));

    ZipFile new_merged_zip(tempdir + "new_merged_zip.zip");
    // Test that the invalid manifest.json file was not merged into the zip file.
    ASSERT_EQ(1, new_merged_zip.size());
    // Test that only the valid manifest.json exists.
    ASSERT_EQ("main/manifest.json", new_merged_zip[0].name);
}

// test that adding multiple manifests result in only one manifest.json being embedded correctly.
// test that one or more invalid manifest results in failing to embed the one and only manifest.json.
TEST_F(ResourceCompilerTest, testMultipleManifestAdd)
{
    const std::string manifest_json_part_1 = R"({
    "bundle.symbolic_name" : "main",
    "bundle.version" : "0.1.0",
    "bundle.activator" : true
    })";

    const std::string manifest_json_part_2 = R"({
    "bundle.description" : "second manifest part"
    })";

    const std::string manifest_json_part_3 = R"({
    "test" : { 
        "foobar" : [1, 2, 5, 7]
     },
    "bundle.foo" : "foo",
    "bundle.bar" : "bar",
    "bundle.name" : "baz"
    })";

    createManifestFile(tempdir, manifest_json_part_1, "manifest_part1.json");
    createManifestFile(tempdir, manifest_json_part_2, "manifest_part2.json");
    createManifestFile(tempdir, manifest_json_part_3, "manifest_part3.json");

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "main";
    cmd << " --out-file \"" << tempdir << "merged_zip.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part1.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part2.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part3.json\"";
    // Test successful concatenation of multiple manifest.json files into one.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    ZipFile merged_zip(tempdir + "merged_zip.zip");
    // Test that the manifest.json file parts were merged into one manifest.json.
    ASSERT_EQ(2, merged_zip.size());
    // Test that only one manifest.json was embedded.
    ASSERT_EQ("main/manifest.json", merged_zip[0].name);

    std::string invalidManifestPart(R"(
		{
		 "invalid": "no comma" 
		 "never.getting" : "here"
		} 
    )");

    createManifestFile(tempdir, invalidManifestPart, "invalid_manifest.json");

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "main";
    cmd << " --out-file \"" << tempdir << "invalid_merged_zip.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part1.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part2.json\"";
    cmd << " --manifest-add \"" << tempdir << "invalid_manifest.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part3.json\"";
    // Test that an invalid manifest json part fails to embed the manifest.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd.str()));

    ZipFile invalid_merged_zip(tempdir + "invalid_merged_zip.zip");
    // Test that no manifest.json was embedded since there was an invalid manifest part.
    ASSERT_EQ(0, invalid_merged_zip.size());

    const std::string manifest_json_part_3_duplicate = R"({
    "test" : { 
        "foobar" : [1, 2, 5, 7]
    },
    "embedded" : { 
        "object" : { 
            "array" : ["one", "two", "twenty"],
            "string" : "boo",
            "integer" : 42,
            "float" : 101.1,
            "boolean" : false,
            "nullvalue" : null
        } 
    },
    "name" : " ",
    "nullvalue" : null
    })";

    createManifestFile(tempdir, manifest_json_part_3_duplicate, "duplicate_manifest.json");

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "main";
    cmd << " --out-file \"" << tempdir << "duplicate_merged_zip.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part1.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part2.json\"";
    cmd << " --manifest-add \"" << tempdir << "duplicate_manifest.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part3.json\"";
    // Test that a duplicate manifest json part fails to embed the manifest.
    ASSERT_EQ(BUNDLE_MANIFEST_VALIDATION_ERROR_CODE, runExecutable(cmd.str()));

    ZipFile duplicate_merged_zip(tempdir + "duplicate_merged_zip.zip");
    // Test that no manifest.json was embedded since there was an duplicate manifest part.
    ASSERT_EQ(0, duplicate_merged_zip.size());
}

// test that specifying multiple manifests concatenates them all into one correctly.
TEST_F(ResourceCompilerTest, testMultipleManifestConcatenation)
{
    const std::string manifest_json_part_1 = R"({
    "bundle.symbolic_name" : "main",
    "bundle.version" : "0.1.0",
    "bundle.activator" : true
    })";

    const std::string manifest_json_part_2 = R"({
    "bundle.description" : "second manifest part"
    })";

    const std::string manifest_json_part_3 = R"({
    "test" : { 
        "foo" : [1, 2, 5, 7],
        "bar" : "baz"
    },
    "embedded" : { 
        "object" : { 
            "array" : ["one", "two", "twenty"],
            "string" : "boo",
            "integer" : 42,
            "float" : 101.1,
            "boolean" : false
        } 
    },
    "name" : " "
    })";

    createManifestFile(tempdir, manifest_json_part_1, "manifest_part1.json");
    createManifestFile(tempdir, manifest_json_part_2, "manifest_part2.json");
    createManifestFile(tempdir, manifest_json_part_3, "manifest_part3.json");

    const std::string manifest_json = R"({
    "bundle.symbolic_name" : "main",
    "bundle.version" : "0.1.0",
    "bundle.activator" : true,
    "bundle.description" : "second manifest part",
    "test" : { 
        "foo" : [1, 2, 5, 7],
        "bar" : "baz"
    },
    "embedded" : { 
        "object" : { 
            "array" : ["one", "two", "twenty"],
            "string" : "boo",
            "integer" : 42,
            "float" : 101.1,
            "boolean" : false
        } 
    },
    "name" : " "
    })";

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "main";
    cmd << " --out-file \"" << tempdir << "merged_zip.zip\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part1.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part2.json\"";
    cmd << " --manifest-add \"" << tempdir << "manifest_part3.json\"";
    // Test the successful concatenation of multiple manifest.json files into one.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    Json::Reader reader;
    Json::Value root;

    // Test that the expected JSON content was parsed correctly.
    ASSERT_TRUE(reader.parse(manifest_json, root, false));

    std::string expectedJSON(root.toStyledString());
    // retrieve the JSON which was concatenated by usResourceCompiler
    std::string concatenatedJSON;
    concatenatedJSON = getManifestContent(tempdir + "merged_zip.zip", "main");

    // line feed and new line characters may be lurking in the strings. I don't know how to use miniz and jsoncpp to
    // embed these characters in a consistent manner, so I'm opting to remove them afterwards.
    removeLineEndings(concatenatedJSON);
    removeLineEndings(expectedJSON);

    // Test that the concatenated JSON content matches the expected JSON content.
    ASSERT_EQ(0, concatenatedJSON.compare(expectedJSON));
}

// test adding a manifest and merging a zip file with a manifest containing a null terminator works.
// This test ensures that the code which extracts the manifest contents from an archive does not truncate the file due
// to a null terminator. NOTE: The C string null terminator in JSON must be escaped. Otherwise, there is a JSON syntax
// error.
TEST_F(ResourceCompilerTest, testManifestWithNullTerminator)
{
    const std::string manifest_json = R"({
    "bundle.symbolic_name" : "main",
    "test" : {
        "bar" : "baz\\0bar",
        "foo\\0bar" : [1, 2, 5, 7]
    }})";

    const std::string jsonFileName("manifest_with_embedded_null_terminator.json");
    const std::string zipFile("embedded_null_terminator.zip");

    createManifestFile(tempdir, manifest_json, jsonFileName);

    std::ostringstream cmd;
    cmd << rcbinpath;
    cmd << " --bundle-name "
        << "main";
    cmd << " --out-file \"" << tempdir << zipFile << "\"";
    cmd << " --manifest-add \"" << tempdir << jsonFileName << "\"";
    // Test the successful embedding of a manifest containing an embedded null terminator.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    Json::Reader reader;
    Json::Value root;
    // Test that the expected JSON content was parsed correctly.
    ASSERT_TRUE(reader.parse(manifest_json, root, false));

    std::string expectedJSON(root.toStyledString());

    std::string nullTerminatorJSON = getManifestContent(tempdir + "embedded_null_terminator.zip", "main");

    // line feed and new line characters may be lurking in the strings. I don't know how to use miniz and jsoncpp to
    // embed these characters in a consistent manner, so I'm opting to remove them afterwards.
    removeLineEndings(nullTerminatorJSON);
    removeLineEndings(expectedJSON);

    // Test that the JSON content matches the expected JSON content.
    ASSERT_EQ(0, nullTerminatorJSON.compare(expectedJSON));

    const std::string mergedZipFile("merged_null_terminator.zip");

    cmd.str(std::string());
    cmd << rcbinpath;
    cmd << " --out-file \"" << tempdir << mergedZipFile << "\"";
    cmd << " --zip-add \"" << tempdir << zipFile << "\"";
    // Test the successful merging of zip file containing a manifest with an embedded null terminator.
    ASSERT_EQ(EXIT_SUCCESS, runExecutable(cmd.str()));

    nullTerminatorJSON = getManifestContent(tempdir + mergedZipFile, "main");

    // line feed and new line characters may be lurking in the strings. I don't know how to use miniz and jsoncpp to
    // embed these characters in a consistent manner, so I'm opting to remove them afterwards.
    removeLineEndings(nullTerminatorJSON);

    // Test that the JSON content matches the expected JSON content.
    ASSERT_EQ(0, nullTerminatorJSON.compare(expectedJSON));
}
