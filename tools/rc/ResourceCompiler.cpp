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

#if defined(__clang__) || defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include "miniz.h"

#if defined(__clang__) || defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifndef __MINGW32__
#    include <filesystem>
#else
#    include <Shlwapi.h>
// Shlwapi.h pulls in windows.h which defines GetObject as a macro,
// conflicting with rapidjson::Value::GetObject().
#    ifdef GetObject
#        undef GetObject
#    endif
#endif

#ifdef US_HAVE_BOOST_NOWIDE
#    include <boost/nowide/args.hpp>
#    include <boost/nowide/fstream.hpp>
namespace us_nowide = boost::nowide;
#else
#    include <nowide/args.hpp>
#    include <nowide/fstream.hpp>
namespace us_nowide = nowide;
#endif

#include "CLI/CLI.hpp"
#include "cppmicroservices/util/RapidJsonUtils.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

// ---------------------------------------------------------------------------------
// --------------------------    PLATFORM SPECIFIC CODE    -------------------------
// ---------------------------------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    define VC_EXTRALEAN
#    include <windows.h>

    // windows.h defines GetObject as a macro (GetObjectA/GetObjectW),
    // which conflicts with rapidjson::Value::GetObject().
#    ifdef GetObject
#        undef GetObject
#    endif

#    define PATH_SEPARATOR "\\"
static std::string
get_error_str()
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();
    std::string errMsg;
    DWORD rc
        = FormatMessageA((FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS),
                         NULL,
                         dw,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         reinterpret_cast<LPSTR>(&lpMsgBuf),
                         0,
                         NULL);
    // If FormatMessage fails using FORMAT_MESSAGE_ALLOCATE_BUFFER
    // it means that the size of the error message exceeds an internal
    // buffer limit (128 kb according to MSDN) and lpMsgBuf will be
    // uninitialized.
    // Inform the caller that the error message couldn't be retrieved.
    if (rc == 0)
    {
        errMsg = "Failed to retrieve error message.";
    }
    else
    {
        errMsg = reinterpret_cast<LPSTR>(lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
    return errMsg;
}

std::string
us_tempfile()
{
    char szTempFileName[MAX_PATH];
    if (GetTempFileNameA(".", "ZIP", 1, szTempFileName) == 0)
    {
        std::cerr << "Error: " << get_error_str() << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string(szTempFileName);
}

#else

#    include <unistd.h>
#    define PATH_SEPARATOR "/"
static std::string
get_error_str()
{
    return strerror(errno);
}

std::string
us_tempfile()
{
    char temppath[] = "./ZIP_XXXXXX";
    if (mkstemp(temppath) == -1)
    {
        std::cerr << "Error: " << get_error_str() << std::endl;
        exit(EXIT_FAILURE);
    }
    return std::string(temppath);
}

#endif

// ---------------------------------------------------------------------------------
// ------------------------   END PLATFORM SPECIFIC CODE    ------------------------
// ---------------------------------------------------------------------------------

namespace
{

    class InvalidManifest : public std::runtime_error
    {
      public:
        InvalidManifest(std::string const& msg) : std::runtime_error(msg) {}
        InvalidManifest(char const* msg) : std::runtime_error(msg) {}
    };

    using cppmicroservices::rapidjsonutils::toStyledString;

    /*
     * @brief parses json content and returns the parsed json or throws.
     * @param jsonContent json content to parse as a std::istream.
     * @param root The parsed Json root object.
     * @throw InvalidManifest if the json is invalid. Parse error information is in the exception.
     * If an exception is thrown, the root param is invalid.
     */
    void
    parseAndValidateJson(std::istream& jsonContent, rapidjson::Document& root)
    {
        rapidjson::IStreamWrapper stream(jsonContent);
        root.ParseStream(stream);

        if (root.HasParseError())
        {
            std::string errs = std::string("Offset ") + std::to_string(root.GetErrorOffset()) + ": "
                               + rapidjson::GetParseError_En(root.GetParseError());
            throw InvalidManifest(errs);
        }

        try
        {
            cppmicroservices::rapidjsonutils::checkDuplicateKeys(root);
        }
        catch (std::runtime_error const& e)
        {
            throw InvalidManifest(e.what());
        }
    }

    /*
     * @brief parses json content from a file and returns the parsed json or throws.
     * @param jsonFile path to a json file.
     * @param root The parsed Json root object.
     * @throw InvalidManifest if the json is invalid. Parse error information is in the exception.
     * If an exception is thrown, the root param is invalid.
     */
    void
    parseAndValidateJsonFromFile(std::string const& jsonFile, rapidjson::Document& root)
    {
        try
        {
            us_nowide::ifstream json(jsonFile);
            if (!json.is_open())
            {
                throw std::runtime_error("Could not open file " + jsonFile);
            }
            parseAndValidateJson(json, root);
        }
        catch (InvalidManifest const& e)
        {
            std::string exceptionMsg(jsonFile + ": " + e.what());
            throw InvalidManifest(exceptionMsg);
        }
    }

    /*
     * @brief extracts a manifest file from the zip archive and checks for correct JSON syntax.
     * @param zipArchive miniz data structure representing the opened zip archive.
     * @param archiveFileName file path of the zip archive.
     * @param archiveEntry archive entry path of the manifest file.
     * @throw InvalidManifest if the json is invalid. Parse error information is in the exception.
     * @throw runtime_error if the manifest file could not be read from the archive.
     */
    void
    validateManifestInArchive(mz_zip_archive* zipArchive,
                              std::string const& archiveFile,
                              std::string const& archiveEntry)
    {
        size_t length = 0;
        void* data = mz_zip_reader_extract_file_to_heap(zipArchive, archiveEntry.c_str(), &length, 0);
        std::unique_ptr<void, void (*)(void*)> manifestFileContents(data, ::free);

        if (!manifestFileContents)
        {
            throw std::runtime_error("Failed to extract " + archiveEntry + " from " + archiveFile);
        }

        try
        {
            rapidjson::Document root;
            std::istringstream json(std::string(reinterpret_cast<char const*>(manifestFileContents.get()), length));
            parseAndValidateJson(json, root);
        }
        catch (InvalidManifest const& e)
        {
            std::string exceptionMsg(archiveFile);
            exceptionMsg += " (" + archiveEntry + ") : " + e.what();
            throw InvalidManifest(exceptionMsg);
        }
    }

    /*
     * @brief Validate manifest files in an archive.
     * @param archiveFile archive file path
     * @throw InvalidManifest on the first invalid manifest found.
     * @throw runtime_error on the first manifest file which could not be read from the archive.
     */
    void
    validateManifestsInArchive(std::string const& archiveFile)
    {
        mz_zip_archive currZipArchive;
        mz_uint currZipIndex = 0;
        memset(&currZipArchive, 0, sizeof(mz_zip_archive));
        std::clog << "Validating manifests in " << archiveFile << " ... " << std::endl;
        if (!mz_zip_reader_init_file(&currZipArchive, archiveFile.c_str(), 0))
        {
            throw std::runtime_error("Could not initialize zip archive " + archiveFile);
        }
        char archiveName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
        mz_uint numZipIndices = mz_zip_reader_get_num_files(&currZipArchive);
        for (currZipIndex = 0; currZipIndex < numZipIndices; ++currZipIndex)
        {
            mz_uint numBytes
                = mz_zip_reader_get_filename(&currZipArchive, currZipIndex, archiveName, sizeof archiveName);
            std::clog << "\tValidating: " << archiveName << " (from " << archiveFile << ") " << std::endl;
            try
            {
                if (numBytes > 1 && archiveName[numBytes - 2] != '/') // The last character is '\0' in the array
                {
                    std::string archiveEntry(archiveName);
                    if (archiveEntry.find_first_of("/", 0) == archiveEntry.find_last_of("/")
                        && std::string::npos != archiveEntry.find("/manifest.json"))
                    {
                        validateManifestInArchive(&currZipArchive, archiveFile, archiveEntry);
                    }
                }
            }
            catch (std::exception const&)
            {
                mz_zip_reader_end(&currZipArchive);
                throw;
            }
        }
        std::clog << "Finished validating manifest files from " << archiveName << std::endl;
        mz_zip_reader_end(&currZipArchive);
    }

    /*
     * @brief concatenates all manifests checking for invalid syntax and
     * duplicate JSON key names.
     * @param manifests a map containing manifest file paths and their content.
     * @pre-condition each manifest in manifests has already been validated.
     * @throw InvalidManifest if the manifest has invalid syntax or duplicate key names
     * @return valid JSON content
     */
    rapidjson::Document
    AggregateManifestsAndValidate(std::map<std::string, rapidjson::Document>& manifests)
    {
        // Initialize as empty object so HasMember/AddMember work immediately.
        // A default-constructed Document is Null, which does not support object operations.
        rapidjson::Document root(rapidjson::kObjectType);
        auto& allocator = root.GetAllocator();

        for (auto& manifest : manifests)
        {
            for (auto& m : manifest.second.GetObject())
            {
                // concatenating all the root objects together means that
                // duplicate key names only have to be checked for children of
                // each root object. Any other duplicate keys would have been
                // detected when validating each individual JSON file.
                if (!root.HasMember(m.name))
                {
                    // Deep copy key and value into root's allocator. 
                    rapidjson::Value key(m.name, allocator);
                    rapidjson::Value val(m.value, allocator);
                    root.AddMember(key, val, allocator);
                }
                else
                {
                    throw InvalidManifest(
                        std::string("Duplicate key: '") + m.name.GetString() + "' found in " + manifest.first);
                }
            }
        }

        std::clog << "concatenated (pre-validated) json:\n" << toStyledString(root) << std::endl;

        // Any duplicate keys would have been flagged earlier while concatenating all the manifest files.
        // This is a final JSON validation which should only find JSON syntax errors caused by an error in
        // concatenation.
        rapidjson::Document manifestJson;
        std::istringstream json(toStyledString(root));
        parseAndValidateJson(json, manifestJson);

        std::clog << "final (validated) manifest.json:\n" <<  toStyledString(manifestJson) << std::endl;
        return manifestJson;
    }
} // namespace

/*
 *@brief class to represent the zip archive for bundles
 */
class ZipArchive
{
  public:
    ZipArchive(std::string const& archiveFileName, int compressionLevel, std::string const& bundleName);
    virtual ~ZipArchive();
    /*
     * @brief Add manifest.json to this zip archive
     * @param manifest contents of the manifest to add to the zip archive
     * @throw std::runtime exception if failed to add manifest.json
     * @throw InvalidManifest if manifest.json is invalid
     */
    void AddManifestFile(rapidjson::Document const& manifest);

    /*
     * @brief Add a file to this zip archive
     * @throw std::runtime exception if failed to add the resource file
     * @throw InvalidManifest if manifest.json is invalid
     * @param resFileName is the path to the resource to be added
     * @param isManifest indicates if the file is the bundle's manifest
     */
    void AddResourceFile(std::string const& resFileName, bool isManifest = false);

    /*
     * @brief Add all files from another zip archive to this zip archive
     * @throw std::runtime exception if failed to add any of the resources
     * @param archiveFileName is the path to the source archive
     */
    void AddResourcesFromArchive(std::string const& archiveFileName);

    // Remove copy constructor and assignment
    ZipArchive(ZipArchive const&) = delete;
    void operator=(ZipArchive const&) = delete;
    // Remove move constructor and assignment
    ZipArchive(ZipArchive&&) = delete;
    ZipArchive& operator=(ZipArchive&&) = delete;

  private:
    /*
     * @brief Add a directory entry to the zip archive
     * @throw std::runtime exception if failed to add the entry
     */
    void AddDirectory(std::string const& dirName);

    /*
     * @brief Checks whether the archive file entry already
     *        exists in the zip archive. If it does, throw an exception.
     * @throw std::runtime_error if the archive entry already exists.
     */
    void CheckAndAddToArchivedNames(std::string const& archiveEntry);

    void
    PrintErrorAndExit(std::string const& errorMsg)
    {
        std::cerr << errorMsg << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string fileName;
    int compressionLevel;
    std::string bundleName;
    std::unique_ptr<mz_zip_archive> writeArchive;
    std::set<std::string> archivedNames; // list of all the file entries
    std::set<std::string> archivedDirs;  // list of all directory entries
};

ZipArchive::ZipArchive(std::string const& archiveFileName, int compressionLevel, std::string const& bName)
    : fileName(archiveFileName)
    , compressionLevel(compressionLevel)
    , bundleName(bName)
    , writeArchive(new mz_zip_archive())
{
    std::clog << "Initializing zip archive " << fileName << " ..." << std::endl;
    // clear the contents of a outFile if it exists
    us_nowide::ofstream ofile(fileName, us_nowide::ofstream::trunc);
    ofile.close();
    if (!mz_zip_writer_init_file(writeArchive.get(), fileName.c_str(), 0))
    {
        throw std::runtime_error("Internal error, could not init new zip archive");
    }
}

void
ZipArchive::CheckAndAddToArchivedNames(std::string const& archiveEntry)
{
    std::clog << "Adding file " << archiveEntry << " ..." << std::endl;
    // add the current file to the new archive
    if (!archivedNames.insert(archiveEntry).second)
    {
        throw std::runtime_error("A file already exists with the name " + archiveEntry);
    }
}

void
ZipArchive::AddManifestFile(rapidjson::Document const& manifest)
{
    // Check to make sure that the bundleName passed on the command line and the
    // bundle.symbolic_name in the manifest file match. If the bundleName is not passed in on the
    // command line, use the name specified in the manifest. If there's a mismatch or if no
    // bundleName is supplied in either location, it's an error.
    // RapidJSON has no .get(key, default) — check HasMember first, then access.
    if (manifest.HasMember("bundle.symbolic_name"))
    {
        auto const& bname = manifest["bundle.symbolic_name"];
        if (bname.IsString())
        {
            std::string bnameStr = bname.GetString();
            if (!bundleName.empty())
            {
                if (bnameStr != bundleName)
                {
                    throw std::runtime_error("Bundle name in manifest " + bnameStr
                                             + " does not match value supplied on command line " + bundleName);
                }
            }
            else
            {
                bundleName = bnameStr;
            }
        }
    }
    if (bundleName.empty())
    {
        throw std::runtime_error(
            "Bundle name is required. Make sure that \"bundle.symbolic_name\" is set in the manifest.json file.");
    }
    std::string styledManifestJson(toStyledString(manifest));
    std::string archiveEntry(bundleName + "/manifest.json");

    // Issue 161.1: Check for file exists first and throw a more desriptive runtime error
    CheckAndAddToArchivedNames(archiveEntry);

    if (MZ_FALSE
        == mz_zip_writer_add_mem(writeArchive.get(),
                                 archiveEntry.c_str(),
                                 styledManifestJson.c_str(),
                                 styledManifestJson.size(),
                                 compressionLevel))
    {
        throw std::runtime_error("Error writing manifest.json to archive " + fileName);
    }
    AddDirectory(bundleName + "/");
}

void
ZipArchive::AddResourceFile(std::string const& resFileName, bool isManifest)
{
    std::string archiveName = resFileName;

    bool pathToResIsAbsolute = false;
#ifndef __MINGW32__
    // Issue 161.3: check to see if resFileName is relative or not, and exit early if it is not.
    std::filesystem::path pathToResFile { resFileName };
    pathToResIsAbsolute = pathToResFile.is_absolute();
#else
    pathToResIsAbsolute = !PathIsRelativeA(resFileName.c_str());
#endif

    if (pathToResIsAbsolute)
    {
        throw std::runtime_error("Relatvie path to resource file required. " + resFileName + " is absolute");
    }

    // This check exists solely to maintain a deprecated way of adding manifest.json
    // through the --res-add option.
    if (isManifest || resFileName == std::string("manifest.json"))
    {
        rapidjson::Document root;
        parseAndValidateJsonFromFile(resFileName, root);
    }

    // if it is a manifest file, we ignore the parent directory path because the
    // manifest file is always placed at the root of the bundle name directory
    if (isManifest && resFileName.find_last_of(PATH_SEPARATOR) != std::string::npos)
    {
        archiveName = resFileName.substr(resFileName.find_last_of(PATH_SEPARATOR) + 1);
    }

    std::string archiveEntry = bundleName + "/" + archiveName;
    CheckAndAddToArchivedNames(archiveEntry);

    if (!mz_zip_writer_add_file(writeArchive.get(),
                                archiveEntry.c_str(),
                                resFileName.c_str(),
                                NULL,
                                0,
                                compressionLevel))
    {
        throw std::runtime_error("Error writing file to archive");
    }
    // add a directory entries for the file path
    size_t lastPathSeparatorPos = archiveEntry.find("/", 0);
    while (lastPathSeparatorPos != std::string::npos)
    {
        AddDirectory(archiveEntry.substr(0, lastPathSeparatorPos + 1));
        lastPathSeparatorPos = archiveEntry.find("/", lastPathSeparatorPos + 1);
    }
}

void
ZipArchive::AddDirectory(std::string const& dirName)
{
    assert(dirName[dirName.length() - 1] == '/');
    if (archivedDirs.insert(dirName).second)
    {
        std::clog << "\t new dir entry " << dirName << std::endl;
        // The directory entry does not yet exist, so add it
        if (!mz_zip_writer_add_mem(writeArchive.get(), dirName.c_str(), NULL, 0, MZ_NO_COMPRESSION))
        {
            throw std::runtime_error("zip add_mem error");
        }
    }
}

ZipArchive::~ZipArchive()
{
    assert(writeArchive->m_zip_mode != MZ_ZIP_MODE_INVALID);
    std::clog << "Finalizing the zip archive ..." << std::endl;
    std::clog << "Archive has the following files" << std::endl;
    for (auto fileNameEntry : archivedNames)
    {
        std::clog << "\t " << fileNameEntry << std::endl;
    }
    std::clog << "and directory entries" << std::endl;
    for (auto dirEntry : archivedDirs)
    {
        std::clog << "\t " << dirEntry << std::endl;
    }
    if (mz_zip_writer_finalize_archive(writeArchive.get()) == MZ_FALSE)
    {
        PrintErrorAndExit("Failed to finalize Zip archive");
    }
    // check state after finalizing the archive.
    assert(writeArchive->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED);
    if (mz_zip_writer_end(writeArchive.get()) == MZ_FALSE)
    {
        PrintErrorAndExit("Failed to close Zip archive");
    }
    // check state after closing the archive file.
    assert(writeArchive->m_zip_mode == MZ_ZIP_MODE_INVALID);
}

void
ZipArchive::AddResourcesFromArchive(std::string const& archiveFileName)
{
    mz_zip_archive currZipArchive;
    mz_uint currZipIndex = 0;
    memset(&currZipArchive, 0, sizeof(mz_zip_archive));
    std::clog << "Merging zip file " << archiveFileName << " ... " << std::endl;
    if (!mz_zip_reader_init_file(&currZipArchive, archiveFileName.c_str(), 0))
    {
        throw std::runtime_error("Could not initialize zip archive " + archiveFileName);
    }
    char archiveName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
    mz_uint numZipIndices = mz_zip_reader_get_num_files(&currZipArchive);
    for (currZipIndex = 0; currZipIndex < numZipIndices; ++currZipIndex)
    {
        mz_uint numBytes = mz_zip_reader_get_filename(&currZipArchive, currZipIndex, archiveName, sizeof archiveName);
        std::clog << "\tmerging: " << archiveName << " (from " << archiveFileName << ") " << std::endl;
        try
        {
            if (numBytes > 1)
            {
                // Issue 161.2: change to use mz_zip_read_is_file_a_directory() instead of checking
                // for the format of the string.
                if (MZ_FALSE == mz_zip_reader_is_file_a_directory(&currZipArchive, currZipIndex))
                {
                    if (!archivedNames.insert(archiveName).second)
                    {
                        throw std::runtime_error("Found duplicate file with name " + std::string(archiveName));
                    }

                    std::string archiveEntry(archiveName);
                    if (archiveEntry.find_first_of("/", 0) == archiveEntry.find_last_of("/")
                        && std::string::npos != archiveEntry.find("/manifest.json"))
                    {
                        validateManifestInArchive(&currZipArchive, archiveFileName, archiveEntry);
                    }

                    if (!mz_zip_writer_add_from_zip_reader(writeArchive.get(), &currZipArchive, currZipIndex))
                    {
                        throw std::runtime_error("Failed to append file " + std::string(archiveName) + " from archive "
                                                 + archiveFileName);
                    }
                }
                else
                {
                    AddDirectory(std::string(archiveName));
                }
            }
        }
        catch (std::exception const&)
        {
            mz_zip_reader_end(&currZipArchive);
            throw;
        }
    }
    std::clog << "Finished merging files from " << archiveFileName << std::endl;
    mz_zip_reader_end(&currZipArchive);
}

// $TODO We need to get the executable name at runtime
#define US_PROG_NAME "usResourceCompiler3"

int
main(int argc, char** argv)
{
    // Deterministic build things to set
#if (defined(_WIN32) || defined(_WIN64)) && defined(US_USE_DETERMINISTIC_BUNDLE_BUILDS)
    setlocale(LC_ALL, "C.UTF-8");
#endif

    us_nowide::args _(argc, argv);

    int const BUNDLE_MANIFEST_VALIDATION_ERROR_CODE(2);
    constexpr int MIN_COMPRESSION_LEVEL = 0;
    constexpr int MAX_COMPRESSION_LEVEL = 9;
    int compressionLevel = MZ_DEFAULT_LEVEL; // default compression level
    int return_code = EXIT_SUCCESS;
    std::string bundleName;

    // CLI11 options
    CLI::App app { "usResourceCompiler3: Resource and manifest zip utility" };

    bool verbose = false;
    app.add_flag("-V,--verbose", verbose, "Run in verbose mode");

    std::string cli_bundleName;
    auto bundle_name_opt = app.add_option("-n,--bundle-name",
                                          cli_bundleName,
                                          "The bundle name as specified in the US_BUNDLE_NAME compile definition.");

    int cli_compressionLevel = MZ_DEFAULT_LEVEL;
    app.add_option("-c,--compression-level",
                   cli_compressionLevel,
                   "Compression level used for zip. Value range is 0 to 9. Default value is 6.")
        ->check(CLI::Range(MIN_COMPRESSION_LEVEL, MAX_COMPRESSION_LEVEL));

    std::string outFile;
    auto out_file_opt = app.add_option("-o,--out-file",
                                       outFile,
                                       "Path to output zip file. If the file exists it will be overwritten. If this "
                                       "option is not provided, a temporary zip file will be created.");

    std::vector<std::string> resAdd;
    app.add_option("-r,--res-add", resAdd, "Path to a resource file, relative to the current working directory.")
        ->take_all();

    std::vector<std::string> zipAdd;
    app.add_option("-z,--zip-add",
                   zipAdd,
                   "Path to a file containing a zip archive to be merged into the output zip file.")
        ->take_all();

    std::vector<std::string> manifestAdd;
    app.add_option("-m,--manifest-add",
                   manifestAdd,
                   "Path to a bundle manifest file. Multiple bundle manifests will be concatenated together into one.")
        ->take_all();

    std::string bundleFile;
    auto bundle_file_opt
        = app.add_option("-b,--bundle-file",
                         bundleFile,
                         "Path to the bundle binary. The resources zip file will be appended to this binary.");

    app.set_help_all_flag("--help-all", "Expand all help");

    try
    {
        app.parse(argc, argv);
    }
    catch (CLI::CallForHelp const& e)
    {
        // When help is requested, print it and return success
        std::cout << app.help() << std::endl;
        return EXIT_SUCCESS;
    }
    catch (CLI::ParseError const& e)
    {
        // Print the error message to stderr for user visibility
        std::cerr << "Error: " << e.what() << std::endl;
        // Always return EXIT_FAILURE for backward compatibility
        return EXIT_FAILURE;
    }

    // If not in verbose mode, suppress the clog stream
    if (!verbose)
    {
        std::clog.setstate(std::ios_base::failbit);
    }

    // Option mapping
    if (bundle_name_opt->count() > 0)
    {
        bundleName = cli_bundleName;
    }

    if (app.count("--compression-level"))
    {
        compressionLevel = cli_compressionLevel;
    }

    // CLI11 does not distinguish between missing and empty string for single-value options,
    // so we need to check count() to see if they were set.
    bool has_bundle_file = bundle_file_opt->count() > 0;
    bool has_out_file = out_file_opt->count() > 0;

    // ---- Argument Consistency Checks ----

    // At least one of --bundle-file or --out-file is required.
    if (!has_bundle_file && !has_out_file)
    {
        std::cerr << "At least one of the options (--bundle-file | --out-file) is required. Check usage." << std::endl;
        return EXIT_FAILURE;
    }

    // Only one --bundle-file, --out-file, or --bundle-name allowed
    if (bundle_file_opt->count() > 1)
    {
        std::cerr << "--bundle-file appears multiple times in the arguments. Check usage." << std::endl;
        return EXIT_FAILURE;
    }
    if (out_file_opt->count() > 1)
    {
        std::cerr << "--out-file appears multiple times in the arguments. Check usage." << std::endl;
        return EXIT_FAILURE;
    }
    if (bundle_name_opt->count() > 1)
    {
        std::cerr << "--bundle-name appears multiple times in the arguments. Check usage." << std::endl;
        return EXIT_FAILURE;
    }

    // If --res-add is given, --bundle-name must also be given.
    if (!resAdd.empty() && bundleName.empty())
    {
        std::cerr << "If --res-add is provided, --bundle-name must be provided." << std::endl;
        return EXIT_FAILURE;
    }

    // Generate a warning that --bundle-name is not necessary in following invocation.
    if (!resAdd.size() && bundle_name_opt->count() && return_code != EXIT_FAILURE)
    {
        std::clog << "Warning: --bundle-name option is unnecessary here." << std::endl;
    }

    // ---- Main Logic ----

    std::string zipFile;
    bool deleteTempFile = false;

    try
    {
        // Append mode only works with one zip-add argument and no res-add/manifest-add
        if (resAdd.empty() && manifestAdd.empty() && zipAdd.size() == 1 && has_bundle_file)
        {
            // jump to append part
            zipFile = zipAdd[0];
        }
        else
        {
            if (has_out_file)
            {
                zipFile = outFile;
            }
            else
            {
                zipFile = us_tempfile();
                deleteTempFile = true;
            }

            std::unique_ptr<ZipArchive> zipArchive(new ZipArchive(zipFile, compressionLevel, bundleName));

            // map of manifest file to its JSON data.
            // std::map ensures manifests are processed in sorted filename order,
            // giving deterministic key ordering in the merged output.
            // rapidjson::Document is move-only, so we use emplace + std::move below.
            std::map<std::string, rapidjson::Document> manifests;

            // Add the manifest file to zip archive
            if (!manifestAdd.empty())
            {
                for (auto const& mfile : manifestAdd)
                {
                    rapidjson::Document manifest;
                    parseAndValidateJsonFromFile(mfile, manifest);
                    bool result;
                    std::tie(std::ignore, result) = manifests.emplace(mfile, std::move(manifest));
                    if (!result)
                    {
                        std::clog << "Skipping duplicate manifest file " << mfile << std::endl;
                    }
                }
                // concatenate all manifest files into one, validate it and add it to the zip archive.
                zipArchive->AddManifestFile(AggregateManifestsAndValidate(manifests));
            }

            // deduplicate resource files
            std::set<std::string> resAddArgs(resAdd.begin(), resAdd.end());
            for (auto const& res : resAddArgs)
            {
                zipArchive->AddResourceFile(res);
            }

            // deduplicate zip archives
            std::set<std::string> resAddArchiveArgs(zipAdd.begin(), zipAdd.end());
            for (auto const& res : resAddArchiveArgs)
            {
                zipArchive->AddResourcesFromArchive(res);
            }
        }

        // ---------------------------------------------------------------------------------
        //      APPEND ZIP to BINARY if bundle-file is specified
        // ---------------------------------------------------------------------------------
        if (has_bundle_file)
        {
            validateManifestsInArchive(zipFile);
            std::string bundleBinaryFile(bundleFile);
            us_nowide::ofstream outFileStream(bundleBinaryFile, std::ios::ate | std::ios::binary | std::ios::app);
            us_nowide::ifstream zipFileStream(zipFile, std::ios::in | std::ios::binary);
            if (outFileStream.is_open() && zipFileStream.is_open())
            {
                std::clog << "Appending file " << bundleBinaryFile << " with contents of resources zip file at "
                          << zipFile << std::endl;
                std::clog << "  Initial file size : " << outFileStream.tellp() << std::endl;
                outFileStream << zipFileStream.rdbuf();
                std::clog << "  Final file size : " << outFileStream.tellp() << std::endl;
                outFileStream.close();
                if (outFileStream.rdstate() & us_nowide::ofstream::failbit)
                {
                    std::cerr << "Failed to write file : " << bundleBinaryFile << std::endl;
                    return_code = EXIT_FAILURE;
                }
            }
            else
            {
                std::cerr << "Opening file " << (outFileStream.is_open() ? zipFile : bundleBinaryFile) << " failed"
                          << std::endl;
                return_code = EXIT_FAILURE;
            }
        }
    }
    catch (InvalidManifest const& ex)
    {
        std::cerr << "JSON Parsing Error: " << ex.what() << std::endl;
        return_code = BUNDLE_MANIFEST_VALIDATION_ERROR_CODE;
    }
    catch (std::exception const& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return_code = EXIT_FAILURE;
    }

    // delete temporary file and report error on failure
    if (deleteTempFile && (std::remove(zipFile.c_str()) != 0))
    {
        std::cerr << "Error removing temporary zip archive " << zipFile << std::endl;
        return_code = EXIT_FAILURE;
    }

    // clear the failbit set by us
    if (!verbose)
    {
        std::clog.clear();
    }

    return return_code;
}
