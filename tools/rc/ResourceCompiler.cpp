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

#include "miniz.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>

#include "optionparser.h"

// ---------------------------------------------------------------------------------
// --------------------------    PLATFORM SPECIFIC CODE    -------------------------
// ---------------------------------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#define PATH_SEPARATOR "\\"
static std::string get_error_str()
{
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();
  std::string errMsg;
  DWORD rc = FormatMessageA((FORMAT_MESSAGE_ALLOCATE_BUFFER |
                             FORMAT_MESSAGE_FROM_SYSTEM |
                             FORMAT_MESSAGE_IGNORE_INSERTS),
                            NULL,
                            dw,
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                            reinterpret_cast<LPTSTR>(&lpMsgBuf),
                            0,
                            NULL );
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
    errMsg = reinterpret_cast<LPCTSTR>(lpMsgBuf);
    LocalFree(lpMsgBuf);
  }
  return errMsg;
}

std::string us_tempfile()
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

#include <unistd.h>
#define PATH_SEPARATOR "/"
static std::string get_error_str()
{
  return strerror(errno);
}

std::string us_tempfile()
{
  char temppath[] = "./ZIP_XXXXXX";
  if(mkstemp(temppath) == -1)
  {
    std::cerr << "Error: " << get_error_str() << std::endl;
    exit(EXIT_FAILURE);
  }
  return std::string(temppath);
}

#endif

/*
 *@brief class to represent the zip archive for bundles
 */
class ZipArchive {
public:
  ZipArchive(const std::string& archiveFileName,
             int compressionLevel,
             const std::string& bundleName);
  virtual ~ZipArchive();
  /*
   * @brief Add a file to this zip archive
   * @throw std::runtime exception if failed to add the resource file
   * @param resFileName is the path to the resource to be added
   * @param isManifest indicates if the file is the bundle's manifest
   */
  void AddResourceFile(const std::string& resFileName, bool isManifest = false);

  /*
   * @brief Add all files from another zip archive to this zip archive
   * @throw std::runtime exception if failed to add any of the resources
   * @param archiveFileName is the path to the source archive
   */
  void AddResourcesFromArchive(const std::string& archiveFileName);

  // Remove copy constructor and assignment
  ZipArchive(const ZipArchive&) = delete;
  void operator=(const ZipArchive&) = delete;
  // Remove move constructor and assignment
  ZipArchive(ZipArchive&&) = delete;
  ZipArchive& operator=(ZipArchive&&) = delete;

private:
  /*
   * @brief Add a directory entry to the zip archive
   * @throw std::runtime exception if failed to add the entry
   */
  void AddDirectory(const std::string& dirName);

  std::string fileName;
  int compressionLevel;
  std::string bundleName;
  std::unique_ptr<mz_zip_archive> writeArchive;
  std::set<std::string> archivedNames;          // list of all the file entries
  std::set<std::string> archivedDirs;           // list of all directory entries
};

ZipArchive::ZipArchive(const std::string& archiveFileName,
                       int compressionLevel,
                       const std::string& bName)
: fileName(archiveFileName)
, compressionLevel(compressionLevel)
, bundleName(bName)
, writeArchive(new mz_zip_archive())
{
  if (bundleName.empty())
  {
    throw std::runtime_error("No bundle name specified");
  }
  std::clog << "Initializing zip archive "  << fileName << " ..." << std::endl;
  // clear the contents of a outFile if it exists
  std::ofstream ofile(fileName, std::ofstream::trunc);
  ofile.close();
  if (!mz_zip_writer_init_file(writeArchive.get(), fileName.c_str(), 0))
  {
    throw std::runtime_error("Internal error, could not init new zip archive");
  }
}

void ZipArchive::AddResourceFile(const std::string& resFileName,
                                 bool isManifest)
{
  std::string archiveName = resFileName;
  // if it is a manifest file, we ignore the parent directory path
  // manifest file is always placed at the root of the bundle name directory
  if (isManifest && resFileName.find_last_of(PATH_SEPARATOR) != std::string::npos)
  {
    archiveName = resFileName.substr(resFileName.find_last_of(PATH_SEPARATOR)+1);
  }

  std::string archiveEntry = bundleName + "/" + archiveName;
  std::clog << "Adding file " << archiveEntry << " ..." << std::endl;
  // add the current file to the new archive
  if (!archivedNames.insert(archiveEntry).second)
  {
    throw std::runtime_error("A file already exists with this name");
  }

  if (!mz_zip_writer_add_file(writeArchive.get(), archiveEntry.c_str(), resFileName.c_str(), NULL,
                              0, compressionLevel))
  {
    throw std::runtime_error("Error writing file to archive");
  }
  // add a directory entries for the file path
  size_t lastPathSeparatorPos = archiveEntry.find("/", 0);
  while(lastPathSeparatorPos != std::string::npos)
  {
    AddDirectory(archiveEntry.substr(0,lastPathSeparatorPos+1));
    lastPathSeparatorPos = archiveEntry.find("/", lastPathSeparatorPos+1);
  }
}

void ZipArchive::AddDirectory(const std::string& dirName)
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

void PrintErrorAndExit(const std::string& errorMsg)
{
  std::cerr << errorMsg << std::endl;
  exit(EXIT_FAILURE);
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
  if(mz_zip_writer_end(writeArchive.get()) == MZ_FALSE)
  {
    PrintErrorAndExit("Failed to close Zip archive");
  }
  // check state after closing the archive file.
  assert(writeArchive->m_zip_mode == MZ_ZIP_MODE_INVALID);
}

void ZipArchive::AddResourcesFromArchive(const std::string &archiveFileName)
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
    std::clog << "\tmerging: " << archiveName << " (from " << archiveFileName << ") "<< std::endl;
    try
    {
      if (numBytes > 1)
      {
        if (archiveName[numBytes-2] != '/') // The last character is '\0' in the array
        {
          if (!archivedNames.insert(archiveName).second)
          {
            throw std::runtime_error("Found duplicate file with name " + std::string(archiveName));
          }
          if (!mz_zip_writer_add_from_zip_reader(writeArchive.get(), &currZipArchive, currZipIndex))
          {
            throw std::runtime_error("Failed to append file " + std::string(archiveName) + "from archive " + archiveFileName);
          }
        }
        else
        {
          AddDirectory(std::string(archiveName));
        }
      }
    }
    catch(const std::exception&)
    {
      mz_zip_reader_end(&currZipArchive);
      throw;
    }
  }
  std::clog << "Finished merging files from " << archiveFileName << std::endl;
  mz_zip_reader_end(&currZipArchive);
}

struct Custom_Arg : public option::Arg
{
  static void printError(const std::string& msg1,
                         const option::Option& opt,
                         const std::string& msg2)
  {
    std::cerr << "ERROR: " << msg1 << opt.name << msg2 << std::endl;
  }

  static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
  {
    if (option.arg != 0 && option.arg[0] != 0)
    {
      return option::ARG_OK;
    }
    if (msg)
    {
      printError("Option '", option, "' requires a non-empty argument\n");
    }
    return option::ARG_ILLEGAL;
  }

  static option::ArgStatus Numeric(const option::Option& option, bool msg)
  {
    char* endptr = nullptr;
    if (option.arg != 0 &&
        strtol(option.arg, &endptr, 10))
    {
      assert(endptr != nullptr);
      return option::ARG_OK;
    }

    if (msg)
    {
      printError("Option '", option, "' requires a numeric argument\n");
    }
    return option::ARG_ILLEGAL;
  }
};

#define US_PROG_NAME "usResourceCompiler"

enum  OptionIndex
{
  UNKNOWN,
  HELP,
  VERBOSE,
  BUNDLENAME,
  COMPRESSIONLEVEL,
  OUTFILE,
  RESADD,
  ZIPADD,
  MANIFESTADD,
  BUNDLEFILE
};

const option::Descriptor usage[] =
{
  {UNKNOWN,          0, "" , ""                 , Custom_Arg::None    , "\nUSAGE: " US_PROG_NAME " [options]\n\n" "Options:" },
  {HELP,             0, "h", "help"             , Custom_Arg::None    , " --help, -h  \tPrint usage and exit." },
  {VERBOSE,          0, "V", "verbose"          , Custom_Arg::None    , " --verbose, -V  \tRun in verbose mode." },
  {BUNDLENAME,       0, "n", "bundle-name"      , Custom_Arg::NonEmpty, " --bundle-name, -n \tThe bundle name as specified in the US_BUNDLE_NAME compile definition."},
  {COMPRESSIONLEVEL, 0, "c", "compression-level", Custom_Arg::Numeric , " --compression-level, -c  \tCompression level used for zip. Value range is 0 to 9. Default value is 6." },
  {OUTFILE,          0, "o", "out-file"         , Custom_Arg::NonEmpty, " --out-file, -o \tPath to output zip file. If the file exists it will be overwritten. If this option is not provided, a temporary zip fie will be created."},
  {RESADD,           0, "r", "res-add"          , Custom_Arg::NonEmpty, " --res-add, -r \tPath to a resource file, relative to the current working directory."},
  {ZIPADD,           0, "z", "zip-add"          , Custom_Arg::NonEmpty, " --zip-add, -z \tPath to a file containing a zip archive to be merged into the output zip file. "},
  {MANIFESTADD,      0, "m", "manifest-add"     , Custom_Arg::NonEmpty, " --manifest-add, -m \tPath to the bundle's manifest file. "},
  {BUNDLEFILE,       0, "b", "bundle-file"      , Custom_Arg::NonEmpty, " --bundle-file, -b \tPath to the bundle binary. The resources zip file will be appended to this binary. "},
  {UNKNOWN,          0, "" ,  ""                , Custom_Arg::None    , "\nExamples:\n\nCreate a zip file with resources\n" "  " US_PROG_NAME " --compression-level 9 --verbose --bundle-name mybundle --out-file Example.zip --manifest-add manifest.json --zip-add filetomerge.zip\n" },
  {UNKNOWN,          0, "" ,  ""                , Custom_Arg::None    , "\nAppend a bundle with resources\n""  " US_PROG_NAME " -v -n mybundle -b mybundle.dylib -m manifest.json -z archivetomerge.zip\n" },
  {UNKNOWN,          0, "" ,  ""                , Custom_Arg::None    , "\nAppend a bundle binary with existing zip file\n" "  " US_PROG_NAME ".exe -b mybundle.dll -z archivetoembed.zip\n" },
  {0,0,0,0,0,0}
};

// ---------------------------------------------------------------------------------
// -----------------------------    MAIN ENTRY POINT    ----------------------------
// ---------------------------------------------------------------------------------

int main(int argc, char** argv)
{
  int compressionLevel = MZ_DEFAULT_LEVEL; //default compression level;
  int return_code = EXIT_SUCCESS;
  std::string bundleName;

  argc -= (argc > 0);
  argv += (argc > 0); // skip program name argv[0]
  option::Stats stats(usage, argc, argv);
  std::unique_ptr<option::Option[]> options(new option::Option[stats.options_max]);
  std::unique_ptr<option::Option[]> buffer(new option::Option[stats.buffer_max]);
  option::Parser parse(usage, argc, argv, options.get(), buffer.get());

  if (parse.error())
  {
    std::cerr << "Parsing command line arguments failed. " << std::endl;
    return_code = EXIT_FAILURE;
  }

  if (parse.nonOptionsCount())
  {
    std::clog << "unrecognized options ..." << std::endl;
    for (int i = 0; i < parse.nonOptionsCount(); ++i)
    {
      std::cout << "\t" << parse.nonOption(i) << std::endl;
    }
    return_code = EXIT_FAILURE;
  }

  option::Option* bundleFileOpt = options[BUNDLEFILE];
  if (bundleFileOpt && bundleFileOpt->count() > 1 )
  {
    std::cerr << "(--bundle-file | -b) appear multiple times in the arguments. Check usage." << std::endl;
    return_code = EXIT_FAILURE;
  }

  option::Option* outFileOpt = options[OUTFILE];
  if (outFileOpt && outFileOpt->count() > 1 )
  {
    std::cerr << "(--out-file | -o) appear multiple times in the arguments. Check usage." << std::endl;
    return_code = EXIT_FAILURE;
  }

  if (!bundleFileOpt && !outFileOpt)
  {
    std::cerr << "At least one of the options (--bundle-file | --out-file) is required." << std::endl;
    return_code = EXIT_FAILURE;
  }

  if (argc == 0 || options[HELP] || return_code == EXIT_FAILURE)
  {
    option::printUsage(std::clog, usage);
    return return_code;
  }

  if (options[BUNDLENAME])
  {
    bundleName = options[BUNDLENAME].arg;
  }

  if (!options[VERBOSE])
  {
    // if not in verbose mode, supress the clog stream
    std::clog.setstate(std::ios_base::failbit);
  }

  if (options[COMPRESSIONLEVEL])
  {
    char* endptr = 0;
    compressionLevel = strtol(options[COMPRESSIONLEVEL].arg, &endptr, 10);
  }
  std::clog << "using compression level " << compressionLevel << std::endl;

  std::string zipFile;
  bool deleteTempFile = false;

  try
  {
    // Append mode only works with one zip-add argument. A bundle can only contain one zip blob.
    if (!options[RESADD] && !options[MANIFESTADD] && options[ZIPADD].count() == 1 && options[BUNDLEFILE])
    {
      // jump to append part.
      zipFile = options[ZIPADD].arg;
    }
    else
    {
      if (outFileOpt)
      {
        zipFile = outFileOpt->arg;
      }
      else
      {
        zipFile = us_tempfile();
        deleteTempFile = true;
      }

      std::unique_ptr<ZipArchive> zipArchive(new ZipArchive(zipFile, compressionLevel, bundleName));
      // Add the manifest file to zip archive
      if (options[MANIFESTADD])
      {
        zipArchive->AddResourceFile(options[MANIFESTADD].arg, true);
      }
      // Add resource files to the zip archive
      for (option::Option* resopt = options[RESADD]; resopt; resopt = resopt->next())
      {
        zipArchive->AddResourceFile(resopt->arg);
      }
      // Merge resources from supplied zip archives
      for (option::Option* opt = options[ZIPADD]; opt; opt = opt->next())
      {
        zipArchive->AddResourcesFromArchive(opt->arg);
      }
    }
    // ---------------------------------------------------------------------------------
    //      APPEND ZIP to BINARY if bundle-file is specified
    // ---------------------------------------------------------------------------------
    if (bundleFileOpt)
    {
      std::string bundleBinaryFile(bundleFileOpt->arg);
      std::ofstream outFileStream(bundleBinaryFile, std::ios::ate | std::ios::binary | std::ios::app);
      std::ifstream zipFileStream(zipFile, std::ios::in | std::ios::binary);
      if (outFileStream.is_open() && zipFileStream.is_open())
      {
        std::clog << "Appending file " << bundleBinaryFile << " with contents of resources zip file at " << zipFile << std::endl;
        std::clog << "  Initial file size : " << outFileStream.tellp() << std::endl;
        outFileStream << zipFileStream.rdbuf();
        std::clog << "  Final file size : " << outFileStream.tellp() << std::endl;
        // Depending on the ofstream destructor to close the file may result in a silent
        // file write error. Hence the explicit call to close.
        outFileStream.close();
        if (outFileStream.rdstate() & std::ofstream::failbit)
        {
          std::cerr << "Failed to write file : " << bundleBinaryFile << std::endl;
          return_code = EXIT_FAILURE;
        }
      }
      else
      {
        std::cerr << "Opening file " << (outFileStream.is_open() ? zipFile : bundleBinaryFile) << " failed" << std::endl;
        return_code = EXIT_FAILURE;
      }
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Error: " << ex.what() << std::endl;
    return_code = EXIT_FAILURE;
  }

  // delete temporary file and report error on failure
  if (deleteTempFile && (std::remove(zipFile.c_str()) != 0))
  {
    std::cerr << "Error removing temporary zip archive "  << zipFile << std:: endl;
    return_code = EXIT_FAILURE;
  }

  // clear the failbit set by us
  if (!options[VERBOSE])
  {
    std::clog.clear();
  }

  return return_code;
}

