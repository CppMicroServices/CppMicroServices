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

#include "miniz.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#define US_STR_(x) #x
#define US_STR(x) US_STR_(x)

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

static void exit_perror(mz_zip_archive* writeArchive, const char* desc)
{
  cleanup_archive(writeArchive);
  fprintf(stderr, "error: ");
  perror(desc);
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

static char* get_error_str()
{
  // Retrieve the system error message for the last-error code
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR) &lpMsgBuf,
    0, NULL );

  return lpMsgBuf;
}

static void free_error_str(char* buf)
{
  LocalFree(buf);
}

static char* us_strcpy(char* dest, size_t dest_size, const char* src)
{
  if (strcpy_s(dest, dest_size, src))
  {
    char* err_str = get_error_str();
    exit_printf(NULL, NULL, err_str);
  }
  return dest;
}

static char* us_strncpy(char* dest, size_t dest_size, const char* src, size_t count)
{
  if (strncpy_s(dest, dest_size, src, count))
  {
    char* err_str = get_error_str();
    exit_printf(NULL, NULL, err_str);
  }
  return dest;
}

static FILE* us_fopen(const char* filename, const char* mode)
{
  FILE* file = NULL;
  fopen_s(&file, filename, mode);
  return file;
}

#define US_CWD(b, s) _getcwd(b, s)

#define US_CLOSE _close
#define US_READ _read
#define US_FOPEN us_fopen
#define US_FILENO _fileno

#define US_STRCASECMP _stricmp
#define US_STRCPY us_strcpy
#define US_STRNCPY us_strncpy

#define US_SSCANF sscanf_s

#else

#include <unistd.h>

static char* get_error_str()
{
  return strerror(errno);
}

static void free_error_str(char* buf)
{
}

static char* us_strcpy(char* dest, size_t dest_size, const char* src)
{
  return strcpy(dest, src);
}

static char* us_strncpy(char* dest, size_t dest_size, const char* src, size_t count)
{
  return strncpy(dest, src, count);
}

#define US_CLOSE close
#define US_READ read
#define US_FOPEN fopen
#define US_FILENO fileno

#define US_STRCASECMP strcasecmp
#define US_STRCPY us_strcpy
#define US_STRNCPY us_strncpy

#define US_SSCANF sscanf

#endif

// ---------------------------------------------------------------------------------
// -----------------------------    DEBUGGING STUFF    -----------------------------
// ---------------------------------------------------------------------------------

//#define DEBUG_TRACE

#ifdef DEBUG_TRACE
#define dbg_print printf
#else
static int void_printf(const char* format, ...) { return 0; }
#define dbg_print void_printf
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
    fprintf(stderr, "Could not allocate enough memory (%ld bytes)\n", size);
    abort();
  }
  return p;
}

static int cmpzipindex(const void *i1, const void *i2)
{
  mz_uint index1 = *(const mz_uint*)i1;
  mz_uint index2 = *(const mz_uint*)i2;
  return index1 == index2 ? 0 : (index1 < index2 ? -1 : 1);
}

static int cmpstringp(const void *p1, const void *p2)
{
  return US_STRCASECMP(* (char * const *) p1, * (char * const *) p2);
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
    archivedNames->names = realloc(archivedNames->names, newCapacity * sizeof(char*));
    if (archivedNames->names == NULL)
    {
      fprintf(stderr, "Could not realloc enough memory (%ld bytes)\n", newCapacity);
      abort();
    }
    memset(archivedNames->names + archivedNames->capacity, 0, sizeof(char*) * (newCapacity - archivedNames->capacity));
    archivedNames->capacity = (mz_uint)newCapacity;
  }

  if (archivedNames->names[archivedNames->size] == NULL)
  {
    archivedNames->names[archivedNames->size] = malloc_or_abort(MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE * sizeof(char));
  }
  US_STRCPY(archivedNames->names[archivedNames->size], MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE, archiveName);
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
    fprintf(stderr, "Archive file name '%s' too long (%ld > %ld)", pArchive_name, length-1, sizeof dirName);
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
        dbg_print("-- found new dir entry %s\n", dirName);
        // The directory entry does not yet exist, so add it
        if (!mz_zip_writer_add_mem(pZip, dirName, NULL, 0, MZ_NO_COMPRESSION))
        {
          dbg_print("-- zip add_mem error\n");
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

// ---------------------------------------------------------------------------------
// -----------------------------    MAIN ENTRY POINT    ----------------------------
// ---------------------------------------------------------------------------------

int main(int argc, char** argv)
{
  int compressionLevel = 6;
  int argIndex = 0;
  int bPrintHelp = 0;
  int bAppendMode = 0;

  int errCode = US_OK;

  int mergeFlag = 0;
  const char* zipFile = NULL;
  const char* moduleName = NULL;
  size_t moduleNameLength = 0;

  FILE* zipfileStream = NULL;
  mz_zip_archive writeArchive;

  us_archived_names archivedNames;
  us_archived_names archivedDirs;

  FILE* appendStream = NULL;

  char archiveName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];

  int numZipArgs = 0;
  int* zipArgIndices = NULL;

  mz_zip_archive currFileArchive;

  int zipArgIndex = 0;

  char readBuffer[1024];
  mz_uint numRead = 0;


  // ---------------------------------------------------------------------------------
  //      COMMAND LINE VALIDATION
  // ---------------------------------------------------------------------------------

  if (argc < 4)
  {
    bPrintHelp = 1;
  }
  else if (argv[1][0] == '-')
  {
    if (strlen(argv[1]) == 2)
    {
      if (US_SSCANF(argv[1], "-%1d", &compressionLevel) != 1)
      {
        bPrintHelp = 1;
      }
      else
      {
        argIndex = 1;
        if (argc < 5 || compressionLevel < 0 || compressionLevel > 9) bPrintHelp = 1;
      }
    }
    else if (strcmp(argv[1], "--append") == 0)
    {
      if (argc > 4)
      {
        bPrintHelp = 1;
      }
      bAppendMode = 1;
    }
    else
    {
      bPrintHelp = 1;
    }
  }

  if (bPrintHelp)
  {
    printf("A resource compiler for C++ Micro Services modules\n\n");
    printf("Usage: usResourceCompiler [-#] zipfile modulename [[-a] file...] [-m archive...]\n");
    printf("Usage: usResourceCompiler --append outfile zipfile\n\n");
    printf("Add entries to zipfile and merge archives.\n\n");
    printf("  -# (-0, -1, -2, -3, -4, -5, -6, -7, -8, -9)\n");
    printf("             The Zip compression level. The default compression level is -6.\n");
    printf("  zipfile    The absolute path of the zip file.\n");
    printf("  modulename The module name as specified in the MODULE_NAME compile definition.\n");
    printf("  file       Path to a resource file, relative to the current working directory.\n");
    printf("  archive    Path to a zip archive for merging into zipfile.\n");
    exit(EXIT_SUCCESS);
  }

  if (bAppendMode)
  {
    // Special "append" mode. Just append zipfile to outfile as a binary blob.
    // Open the module file for appending the temporary zip archive
    dbg_print("Opening outfile '%s' as ab... ", argv[2]);
    if (NULL == (appendStream = US_FOPEN(argv[2], "ab")))
    {
      dbg_print("failure\n");
      exit_perror(NULL, "fopen");
    }
    else
    {
      dbg_print("success\n");
    }

    dbg_print("Opening zipfile '%s' as rb... ", argv[3]);
    if (NULL == (zipfileStream = US_FOPEN(argv[3], "rb")))
    {
      dbg_print("failure\n");
      exit_perror(NULL, "fopen");
    }
    else
    {
      dbg_print("success\n");
    }

    dbg_print("Appending zipfile to outfile\n");
    do
    {
      numRead = US_READ(US_FILENO(zipfileStream), readBuffer, sizeof(readBuffer));
      if (numRead == -1)
      {
        exit_perror(NULL, "read");
      }
      fwrite(readBuffer, numRead, 1, appendStream);
      if (ferror(appendStream))
      {
        exit_printf(&writeArchive, "Appending zipfile failed\n");
      }
    } while (numRead != 0);

    fclose(zipfileStream);
    fclose(appendStream);
    exit(EXIT_SUCCESS);
  }


  // ---------------------------------------------------------------------------------
  //      OPEN OR CREATE ZIP FILE
  // ---------------------------------------------------------------------------------

  zipFile = argv[++argIndex];
  moduleName = argv[++argIndex];
  moduleNameLength = strlen(moduleName);

  memset(&writeArchive, 0, sizeof(writeArchive));
  memset(&archivedNames, 0, sizeof archivedNames);
  memset(&archivedDirs, 0, sizeof archivedDirs);


  // ---------------------------------------------------------------------------------
  //      ZIP ARCHIVE WRITING (temporary archive)
  // ---------------------------------------------------------------------------------

  // Create a new zip archive which will be copied to zipfile later
  dbg_print("Creating zip archive\n");
  remove(zipFile);

  if (!mz_zip_writer_init_file(&writeArchive, zipFile, 0))
  {
    exit_printf(&writeArchive, "Internal error, could not init new zip archive\n");
  }
  dbg_print("Initialized zip archive\n");

  // Add current files to the zip archive
  zipArgIndices = malloc_or_abort(argc * sizeof *zipArgIndices);
  while(++argIndex < argc)
  {
    const char* fileName = argv[argIndex];
    const size_t fileNameLength = strlen(fileName);

    // determine the argument type
    if (strcmp(argv[argIndex], "-a") == 0)
    {
      mergeFlag = 0;
      continue;
    }
    else if (strcmp(argv[argIndex], "-m") == 0)
    {
      mergeFlag = 1;
      continue;
    }

    if (mergeFlag)
    {
      // check if the current file is a valid zip archive
      memset(&currFileArchive, 0, sizeof(currFileArchive));
      if (mz_zip_reader_init_file(&currFileArchive, fileName, 0))
      {
        dbg_print("Input is a valid zip archive: %s\n", fileName);
        zipArgIndices[numZipArgs++] = argIndex;
        mz_zip_reader_end(&currFileArchive);
      }
      // silently ignore files which are not zip archives
      continue;
    }

    if (fileNameLength + 1 + moduleNameLength > MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1)
    {
      exit_printf(&writeArchive, "Resource filename too long: %s\n", moduleName);
    }
    US_STRCPY(archiveName, sizeof archiveName, moduleName);
    archiveName[moduleNameLength] = '/';
    US_STRCPY(archiveName + moduleNameLength + 1, (sizeof archiveName) - (moduleNameLength + 1), fileName);

    printf("  adding: %s\n", archiveName);

    // add the current file to the new archive
    if ((errCode = us_zip_writer_add_file(&writeArchive, archiveName, fileName, NULL, 0, compressionLevel,
                                         &archivedNames, &archivedDirs)))
    {
      dbg_print("Adding %s failed\n", archiveName);
      exit_printf(&writeArchive, us_error_msg[errCode], archiveName, fileName);
    }
  }

  us_archived_names_sort(&archivedNames);

  dbg_print("Added cmd line files to zip archive\n");

  // ---------------------------------------------------------------------------------
  //      MERGE ZIPFILE ENTRIES (into temporary archive)
  // ---------------------------------------------------------------------------------

  for (zipArgIndex = 0; zipArgIndex < numZipArgs; ++zipArgIndex)
  {
    mz_zip_archive currZipArchive;
    const char* currArchiveFileName = NULL;
    mz_uint currZipIndex = 0;
    mz_uint numZipIndices = 0;

    memset(&currZipArchive, 0, sizeof(mz_zip_archive));
    currArchiveFileName = argv[zipArgIndices[zipArgIndex]];
    if (!mz_zip_reader_init_file(&currZipArchive, currArchiveFileName, 0))
    {
      exit_printf(&writeArchive, "Could not initialize zip archive %s\n", currArchiveFileName);
    }

    numZipIndices = mz_zip_reader_get_num_files(&currZipArchive);
    for (currZipIndex = 0; currZipIndex < numZipIndices; ++currZipIndex)
    {
      errCode = us_zip_writer_add_from_zip_reader(&writeArchive, &currZipArchive, currZipIndex, &archivedNames,
                                                  &archivedDirs, archiveName, sizeof archiveName);
      printf(" merging: %s (from %s)\n", archiveName, currArchiveFileName);
      if (errCode == US_ARCHIVED_NAMES_ERROR_DUPLICATE)
      {
        printf(" warning: Merge failed: ");
        printf(us_error_msg[errCode], archiveName, currArchiveFileName);
      }
      else if (errCode != US_OK)
      {
        mz_zip_reader_end(&currZipArchive);
        exit_printf(&writeArchive, us_error_msg[errCode], archiveName, currArchiveFileName);
      }
    }

    mz_zip_reader_end(&currZipArchive);
    us_archived_names_sort(&archivedNames);
  }

  // We are finished, finalize the zip archive
  if (!mz_zip_writer_finalize_archive(&writeArchive))
  {
    exit_printf(&writeArchive, "Could not finalize zip archive\n");
  }

  dbg_print("Finalized zip archive\n");


  // ---------------------------------------------------------------------------------
  //      CLEANUP
  // ---------------------------------------------------------------------------------

  free(zipArgIndices);
  us_archived_names_free(&archivedNames);
  us_archived_names_free(&archivedDirs);

  if (cleanup_archive(&writeArchive) == -1)
  {
    fprintf(stderr, "Internal error finalizing zip archive\n");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
