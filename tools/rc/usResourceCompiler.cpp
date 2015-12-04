/*=============================================================================
 
 Library: CppMicroServices
 
 Copyright (c) The CppMicroServices developers. See the COPYRIGHT
 file at the top-level directory of this distribution and at
 https://github.com/saschazelzer/CppMicroServices/COPYRIGHT .
 
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

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

#include "optionparser.h"

static int cleanup_archive(mz_zip_archive* writeArchive)
{
  if (writeArchive && writeArchive->m_zip_mode != MZ_ZIP_MODE_INVALID)
  {
    if (writeArchive->m_zip_mode != MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED)
    {
      if (!mz_zip_writer_finalize_archive(writeArchive))
      {
        return -1;
      }
    }
    if (writeArchive->m_zip_mode != MZ_ZIP_MODE_INVALID)
    {
      if (!mz_zip_writer_end(writeArchive))
      {
        return -1;
      }
    }
  }
  return 0;
}

static void exit_printf(mz_zip_archive* writeArchive, const char* format, ...)
{
  va_list args;
  cleanup_archive(writeArchive);
  fprintf(stderr, "error: ");
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

// ---------------------------------------------------------------------------------
// --------------------------    PLATFORM SPECIFIC CODE    -------------------------
// ---------------------------------------------------------------------------------

#if defined(_WIN32) || defined(_WIN64)

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <io.h>
#include <direct.h>
#include <share.h>
#include <fcntl.h>
#include <sys/stat.h>

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
                            (LPTSTR) &lpMsgBuf,
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

static char* us_strncpy(char* dest, size_t dest_size, const char* src, size_t count)
{
  if (strncpy_s(dest, dest_size, src, count))
  {
    exit_printf(NULL, NULL, get_error_str().c_str());
  }
  return dest;
}

static FILE* us_fopen(const char* filename, const char* mode)
{
  FILE* file = NULL;
  fopen_s(&file, filename, mode);
  return file;
}

std::string us_tempfile()
{
  char szTempFileName[MAX_PATH];
  if (GetTempFileNameA(".", "ZIP", 1, szTempFileName) == 0)
  {
    exit_printf(NULL, NULL, get_error_str().c_str());
  }
  return std::string(szTempFileName);
}

#define US_STRCASECMP _stricmp
#define US_STRNCPY us_strncpy

#else

#include <unistd.h>

static std::string get_error_str()
{
  return strerror(errno);
}

static char* us_strncpy(char* dest, size_t /*dest_size*/, const char* src, size_t count)
{
  return strncpy(dest, src, count);
}

std::string us_tempfile()
{
  char temppath[] = "./ZIP_XXXXXX";
  if(mkstemp(temppath) == -1)
  {
    exit_printf(NULL, NULL, get_error_str().c_str());
  }
  return std::string(temppath);
}

#define US_STRCASECMP strcasecmp
#define US_STRNCPY us_strncpy

#endif

// ---------------------------------------------------------------------------------
// -----------------------------    HELPER FUNCTIONS    ----------------------------
// ---------------------------------------------------------------------------------

void* malloc_or_abort(size_t size)
{
  void* p;
  if (size == 0) size = 1;
  p = malloc(size);
  if (!p)
  {
    // try to print an error message; this might very well fail
    fprintf(stderr, "Could not allocate enough memory (%zd bytes)\n", size);
    abort();
  }
  return p;
}

static int cmpstringp(const void *p1, const void *p2)
{
  return US_STRCASECMP(reinterpret_cast<const char*>(p1), reinterpret_cast<const char*>(p2));
}

typedef struct us_archived_names_tag
{
  char** names;
  mz_uint size;
  mz_uint capacity;
  mz_uint orderedSize;
} us_archived_names;

static void us_archived_names_free(us_archived_names * archivedNames)
{
  mz_uint i;
  for (i = 0; i < archivedNames->size; ++i)
  {
    free(archivedNames->names[i]);
  }
  free(archivedNames->names);
}

enum
{
  US_OK = 0,
  US_ARCHIVED_NAMES_ERROR_DUPLICATE = 1,
  US_MZ_ERROR_ADD_FILE = 2,
  US_ERROR_INVALID = 3
};

// messages can take two arguments:
//   1. The archive entry name
//   2. The path of the zip archive
const char* us_error_msg[] = {
  "ok\n",
  "Duplicate entry '%s' (in %s)\n",
  "Could not add resource %s\n"
};

static int us_archived_names_append(us_archived_names* archivedNames, const char* archiveName)
{
  if (archivedNames->names != NULL &&
      bsearch(&archiveName, archivedNames->names, archivedNames->orderedSize, sizeof(char*), cmpstringp) != NULL)
  {
    return US_ARCHIVED_NAMES_ERROR_DUPLICATE;
  }
  
  if (archivedNames->size >= archivedNames->capacity)
  {
    size_t newCapacity = archivedNames->size > archivedNames->capacity + 100 ? archivedNames->size + 1 : archivedNames->capacity + 100;
    archivedNames->names = reinterpret_cast<char**>(realloc(archivedNames->names, newCapacity * sizeof(char*)));
    if (archivedNames->names == NULL)
    {
      std::cerr << "Could not realloc enough memory " << newCapacity << std::endl;
      abort();
    }
    memset(archivedNames->names + archivedNames->capacity, 0, sizeof(char*) * (newCapacity - archivedNames->capacity));
    archivedNames->capacity = static_cast<mz_uint>(newCapacity);
  }
  
  if (archivedNames->names[archivedNames->size] == NULL)
  {
    archivedNames->names[archivedNames->size] = reinterpret_cast<char*>(malloc_or_abort(MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE * sizeof(char)));
  }
  US_STRNCPY(archivedNames->names[archivedNames->size], MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE, archiveName, strlen(archiveName));
  ++archivedNames->size;
  
  return US_OK;
}

static void us_archived_names_sort(us_archived_names* archivedNames)
{
  qsort(archivedNames->names, archivedNames->size, sizeof(char*), cmpstringp);
  archivedNames->orderedSize = archivedNames->size;
}

static int us_zip_writer_add_dir_entries(mz_zip_archive* pZip, const char* pArchive_name, us_archived_names* archived_dirs)
{
  size_t end;
  size_t length = strlen(pArchive_name);
  char dirName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
  if (sizeof dirName < length - 1)
  {
    // This should be impossible
    fprintf(stderr, "Archive file name '%s' too long (%zd > %zd)", pArchive_name, length-1, sizeof dirName);
    exit(EXIT_FAILURE);
  }
  
  // split the archive name into directory tokens
  for (end = 0; end < length; ++end)
  {
    if (pArchive_name[end] == '/')
    {
      US_STRNCPY(dirName, sizeof dirName, pArchive_name, end + 1);
      //if (end < length-1)
      //{
      dirName[end+1] = '\0';
      //}
      if (us_archived_names_append(archived_dirs, dirName) == US_OK)
      {
        std::clog << "-- found new dir entry " << dirName << std::endl;
        // The directory entry does not yet exist, so add it
        if (!mz_zip_writer_add_mem(pZip, dirName, NULL, 0, MZ_NO_COMPRESSION))
        {
          std::cerr << "-- zip add_mem error" << std::endl;
          return US_MZ_ERROR_ADD_FILE;
        }
        us_archived_names_sort(archived_dirs);
      }
    }
  }
  return US_OK;
}

static int us_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name,
                                  const char *pSrc_filename, const void *pComment,
                                  mz_uint16 comment_size, mz_uint level_and_flags,
                                  us_archived_names* archived_names,
                                  us_archived_names* archived_dirs)
{
  int retCode = us_archived_names_append(archived_names, pArchive_name);
  if (US_OK != retCode) return retCode;
  
  if (!mz_zip_writer_add_file(pZip, pArchive_name, pSrc_filename, pComment,
                              comment_size, level_and_flags))
  {
    return US_MZ_ERROR_ADD_FILE;
  }
  return us_zip_writer_add_dir_entries(pZip, pArchive_name, archived_dirs);
}

static int us_zip_writer_add_from_zip_reader(mz_zip_archive *pZip, mz_zip_archive *pSource_zip,
                                             mz_uint file_index, us_archived_names* archived_names,
                                             us_archived_names* archived_dirs,
                                             char* archiveName, mz_uint archiveNameSize)
{
  int retCode = 0;
  
  mz_uint numBytes = mz_zip_reader_get_filename(pSource_zip, file_index, archiveName, archiveNameSize);
  if (numBytes > 1 && archiveName[numBytes-2] != '/')
  {
    retCode = us_archived_names_append(archived_names, archiveName);
    if (US_OK != retCode) return retCode;
    
    if (!mz_zip_writer_add_from_zip_reader(pZip, pSource_zip, file_index))
    {
      return US_MZ_ERROR_ADD_FILE;
    }
  }
  return us_zip_writer_add_dir_entries(pZip, archiveName, archived_dirs);
}

struct Custom_Arg : public option::Arg
{
  static void printError(const std::string& msg1, const option::Option& opt, const std::string& msg2)
  {
    std::cerr << "ERROR: " << msg1 << opt.name << msg2 << std::endl;
  }
  
  static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
  {
    if (option.arg != 0 && option.arg[0] != 0)
      return option::ARG_OK;
    
    if (msg) printError("Option '", option, "' requires a non-empty argument\n");
    return option::ARG_ILLEGAL;
  }
  
  static option::ArgStatus Numeric(const option::Option& option, bool msg)
  {
    char* endptr = 0;
    if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
    if (endptr != option.arg && *endptr == 0)
      return option::ARG_OK;
    
    if (msg) printError("Option '", option, "' requires a numeric argument\n");
    return option::ARG_ILLEGAL;
  }
};

#define US_PROG_NAME "usResourceCompiler"

enum  OptionIndex { UNKNOWN, HELP, VERBOSE, BUNDLENAME, COMPRESSIONLEVEL, OUTFILE, RESFILE, MERGEZIP, APPENDBINARY };
const option::Descriptor usage[] =
{
  {UNKNOWN,      0, "" , ""    , Custom_Arg::None, "\nUSAGE: " US_PROG_NAME " [options]\n\n"
    "Options:" },
  {HELP,         0, "h" , "help",Custom_Arg::None, " --help, -h  \tPrint usage and exit." },
  {VERBOSE,         0, "v" , "verbose",Custom_Arg::None, " --verbose, -v  \tRun in verbose mode." },
  {BUNDLENAME,    0, "b", "bundle-name", Custom_Arg::NonEmpty, " --bundle-name, -b \tThe bundle name as specified in the BUNDLE_NAME compile definition."},
  {COMPRESSIONLEVEL,  0, "c", "compression-level", Custom_Arg::Numeric, " --compression-level, -c  \tCompression level used for zip . Value range is 0 to 9. Default value is 6." },
  {OUTFILE, 0, "o", "out-file", Custom_Arg::NonEmpty, " --out-file, -o \tPath to output zip file. If the file exists it will be overwritten. If this option is not provided, a temporary zip fie will be created."},
  {RESFILE, 0, "r", "res-file", Custom_Arg::NonEmpty, " --res-file, -r \tPath to a resource file, relative to the current working directory."},
  {MERGEZIP, 0, "m", "merge-zip-file", Custom_Arg::NonEmpty, " --merge-zip-file, -m \tPath to a zip archive for merging into output zip file. "},
  {APPENDBINARY, 0, "a", "append-binary", Custom_Arg::NonEmpty, " --append-binary, -a \tPath to the bundle binary. The resources zip file will be appended to this binary. "},
  {UNKNOWN,      0, "" ,  ""   , Custom_Arg::None, "\nExamples:\n\nCreate a zip file with resources\n"
    "  " US_PROG_NAME " --compression-level 9 --verbose --bundle-name mybundle --out-file Example.zip --res-file manifest.json --merge filetomerge.zip\n" },
  {UNKNOWN,      0, "" ,  ""   , Custom_Arg::None, "\nAppend a bundle with resources\n"
    "  " US_PROG_NAME " -v -b mybundle -a mybundle.dylib -r manifest.json -m archivetomerge.zip\n" },
  {UNKNOWN,      0, "" ,  ""   , Custom_Arg::None, "\nAppend a bundle binary with existing zip file\n"
    "  " US_PROG_NAME ".exe -a mybundle.dll -m archivetoembed.zip\n" },
  {0,0,0,0,0,0}
};

// ---------------------------------------------------------------------------------
// -----------------------------    MAIN ENTRY POINT    ----------------------------
// ---------------------------------------------------------------------------------

int main(int argc, char** argv)
{
  int compressionLevel = 6;
  int errCode = US_OK, return_code = EXIT_SUCCESS;
  std::string zipFile;
  std::string bundleName;
  std::string archiveName;
  
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
  
  option::Option* appendbinaryopt = options[APPENDBINARY];
  if (appendbinaryopt && appendbinaryopt->count() > 1 )
  {
    std::cerr << "(--append-binary | -a) appear multiple times in the arguments. Check usage." << std::endl;
    return_code = EXIT_FAILURE;
  }
  
  option::Option* outfileopt = options[OUTFILE];
  if (outfileopt && outfileopt->count() > 1 )
  {
    std::cerr << "(--out-file | -o) appear multiple times in the arguments. Check usage." << std::endl;
    return_code = EXIT_FAILURE;
  }
  
  if (!appendbinaryopt && !outfileopt)
  {
    std::cerr << "At least one of the options (--apend-binary | --out-file) is required." << std::endl;
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
  
  std::string outfile;
  if (outfileopt)
  {
    outfile = outfileopt->arg;
  }
  else
  {
    outfile = us_tempfile();
  }
  
  // ---------------------------------------------------------------------------------
  //      OPEN OR CREATE ZIP FILE
  // ---------------------------------------------------------------------------------
  
  mz_zip_archive writeArchive;
  us_archived_names archivedNames;
  us_archived_names archivedDirs;
  memset(&writeArchive, 0, sizeof(writeArchive));
  memset(&archivedNames, 0, sizeof archivedNames);
  memset(&archivedDirs, 0, sizeof archivedDirs);
  
  
  // ---------------------------------------------------------------------------------
  //      ZIP ARCHIVE WRITING (temporary archive)
  // ---------------------------------------------------------------------------------
  
  // Create a new zip archive which will be copied to zipfile later
  std::clog << "Creating zip archive " << outfile << std::endl;
  // clear the contents of a outfile if it exists
  std::ofstream ofile(outfile, std::ofstream::trunc);
  ofile.close();
  
  if (!mz_zip_writer_init_file(&writeArchive, outfile.c_str(), 0))
  {
    exit_printf(&writeArchive, "Internal error, could not init new zip archive\n");
  }
  std::clog << "Initialized zip archive" << std::endl;
  
  // check if resources can be added
  option::Option* resopt = options[RESFILE];
  if (resopt && !bundleName.size())
  {
    std::cerr << "No bundle name specified ... cannot add resource files to zip archive" << std::endl;
    return EXIT_FAILURE;
  }
  // Add resource files to the zip archive
  for (; resopt; resopt = resopt->next())
  {
    std::string resArchiveName = resopt->arg;
    resArchiveName.insert(0, "/");
    resArchiveName.insert(0, bundleName);
    
    std::clog << "  adding: " << resArchiveName << std::endl;
    
    // add the current file to the new archive
    if ((errCode = us_zip_writer_add_file(&writeArchive, resArchiveName.c_str(), resopt->arg, NULL, 0, compressionLevel, &archivedNames, &archivedDirs)))
    {
      std::clog << "Adding " << resArchiveName << " failed" << std::endl;
      exit_printf(&writeArchive, us_error_msg[errCode], resArchiveName.c_str(), resopt->arg);
    }
  }
  std::clog << "Finished adding resource files to zip archive" << std::endl;
  us_archived_names_sort(&archivedNames);
  
  // ---------------------------------------------------------------------------------
  //      MERGE ZIPFILE ENTRIES (into temporary archive)
  // ---------------------------------------------------------------------------------
  for (option::Option* opt = options[MERGEZIP]; opt; opt = opt->next())
  {
    mz_zip_archive currZipArchive;
    mz_uint currZipIndex = 0;
    mz_uint numZipIndices = 0;
    memset(&currZipArchive, 0, sizeof(mz_zip_archive));
    std::string currArchiveFileName = opt->arg;
    std::clog << "Merging zip file " << currArchiveFileName << std::endl;
    if (!mz_zip_reader_init_file(&currZipArchive, currArchiveFileName.c_str(), 0))
    {
      exit_printf(&writeArchive, "Could not initialize zip archive %s\n", currArchiveFileName.c_str());
    }
    char archiveName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];
    numZipIndices = mz_zip_reader_get_num_files(&currZipArchive);
    for (currZipIndex = 0; currZipIndex < numZipIndices; ++currZipIndex)
    {
      errCode = us_zip_writer_add_from_zip_reader(&writeArchive, &currZipArchive, currZipIndex, &archivedNames,
                                                  &archivedDirs, archiveName, sizeof archiveName);
      std::clog << " merging: " << archiveName << " (from " << currArchiveFileName << ") "<< std::endl;
      if (errCode == US_ARCHIVED_NAMES_ERROR_DUPLICATE)
      {
        std::clog << " warning: Merge failed: " << std::endl;
        printf(us_error_msg[errCode], archiveName, currArchiveFileName.c_str());
      }
      else if (errCode != US_OK)
      {
        mz_zip_reader_end(&currZipArchive);
        exit_printf(&writeArchive, us_error_msg[errCode], archiveName, currArchiveFileName.c_str());
      }
    }
    std::clog << "Finished merging zip files" << std::endl;
    
    mz_zip_reader_end(&currZipArchive);
    us_archived_names_sort(&archivedNames);
  }
  
  // We are finished, finalize the zip archive
  if (!mz_zip_writer_finalize_archive(&writeArchive))
  {
    exit_printf(&writeArchive, "Could not finalize zip archive\n");
  }
  
  std::clog << "Finalized zip archive" << std::endl;
  
  // ---------------------------------------------------------------------------------
  //      CLEANUP
  // ---------------------------------------------------------------------------------
  
  us_archived_names_free(&archivedNames);
  us_archived_names_free(&archivedDirs);
  
  if (cleanup_archive(&writeArchive) == -1)
  {
    std::cerr << "Internal error finalizing zip archive" << std::endl;
    return EXIT_FAILURE;
  }
  
  // ---------------------------------------------------------------------------------
  //      APPEND ZIP to BINARY if append-binary is specified
  // ---------------------------------------------------------------------------------
  
  if (appendbinaryopt)
  {
    std::string bundleBinaryFile(appendbinaryopt->arg);
    std::ofstream outFileStream(bundleBinaryFile, std::ios::ate | std::ios::binary | std::ios::app);
    std::ifstream zipFileStream(outfile, std::ios_base::binary);
    std::clog << "Appending file " << bundleBinaryFile << " with contents of resources zip file at " << outfile << std::endl;
    outFileStream.seekp(0, std::ios_base::end);
    std::clog << "  Initial file size : " << outFileStream.tellp() << std::endl;
    outFileStream << zipFileStream.rdbuf();
    std::clog << "  Final file size : " << outFileStream.tellp() << std::endl;
  }
  
  
  if (appendbinaryopt && !outfileopt)
  {
    if(std::remove(outfile.c_str()))
      std::cerr << "Error removing temporary zip archive "  << outfile << std:: endl;
    else
      std::clog << "Removed temporary zip archive " << outfile << std:: endl;
  }
  
  // clear the failbit set by us
  if (!options[VERBOSE])
  {
    std::clog.clear();
  }
  
  return return_code;
}

