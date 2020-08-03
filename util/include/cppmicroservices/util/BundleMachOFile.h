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

#if defined (US_PLATFORM_APPLE)

#include "BundleObjFile.h"
#include "DataContainer.h"
#include "MappedFile.h"

#include <cerrno>
#include <cstring>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <fstream>
#include <memory>

#include <sys/stat.h>

namespace cppmicroservices {

struct InvalidMachOException : public InvalidObjFileException
{
  InvalidMachOException(const std::string& what, int errorNumber = 0)
    : InvalidObjFileException(what, errorNumber)
  {}
};

template<uint32_t>
struct MachO;

template<>
struct MachO<MH_MAGIC>
{
  typedef mach_header Mhdr;
  typedef struct nlist symtab_entry;
};

template<>
struct MachO<MH_MAGIC_64>
{
  typedef mach_header_64 Mhdr;
  typedef nlist_64 symtab_entry;
};

template<typename I>
I readBE(I i)
{
#ifdef US_LITTLE_ENDIAN
  I r = 0;
  for (unsigned int n = 0; n < sizeof(I); ++n) {
    r |= static_cast<I>(*(reinterpret_cast<unsigned char*>(&i) + n))
         << ((sizeof(I) - 1 - n) * 8);
  }
  return r;
#else
  return i;
#endif
}

template<class MachOType>
class BundleMachOFile
  : public BundleObjFile
  , private MachOType
{
public:
  typedef typename MachOType::Mhdr Mhdr;
  typedef typename MachOType::symtab_entry symtab_entry;

  BundleMachOFile(std::ifstream& fs, std::size_t fileOffset, const std::string& location)
    : m_rawData()
  {
    fs.seekg(fileOffset);
    Mhdr mhdr;
    fs.read(reinterpret_cast<char*>(&mhdr), sizeof mhdr);
    if (mhdr.filetype != MH_DYLIB && mhdr.filetype != MH_BUNDLE) {
      throw InvalidMachOException(
        "Not a Mach-O dynamic shared library or bundle file.");
    }
      
    fs.seekg(fileOffset + sizeof(mach_header_64));

    // iterate over all load commands
    uint32_t ncmds = mhdr.ncmds;
    uint32_t lcmd_offset = static_cast<uint32_t>(fs.tellg());
    
    for (uint32_t i = 0; i < ncmds; ++i) {
      load_command lcmd;
      fs.read(reinterpret_cast<char*>(&lcmd), sizeof lcmd);
      if (!m_rawData && LC_SEGMENT_64 == lcmd.cmd) {
        m_rawData = GetRawBundleResources<segment_command_64, section_64>(fs, location, fileOffset, lcmd_offset);
      } else if (!m_rawData && LC_SEGMENT == lcmd.cmd) {
        m_rawData = GetRawBundleResources<segment_command, section>(fs, location, fileOffset, lcmd_offset);
      }

      lcmd_offset += lcmd.cmdsize;
      fs.seekg(lcmd_offset);
    }
  }

  template<typename SegmentCommand, typename Section>
  std::shared_ptr<RawBundleResources> GetRawBundleResources(std::ifstream& fs, const std::string& filePath, std::size_t fileOffset, uint32_t lcmd_offset)
  {
    fs.seekg(fileOffset + lcmd_offset);
    SegmentCommand segment;
    fs.read(reinterpret_cast<char*>(&segment), sizeof(SegmentCommand));
    if(0 == strcmp("__TEXT", segment.segname)) {
       // find "us_resources" section
       for (uint32_t i = 0; i < segment.nsects; ++i) {
         Section section;
         fs.read(reinterpret_cast<char*>(&section), sizeof(Section));
         if (0 == strcmp("us_resources", section.sectname) &&
             0 < section.size) {
           // try to be smart about when to use mmap. Benchmark tests show
           // that mmap is slower than std::ifstream::read until the file size is around 10mb.
           constexpr std::size_t zipFileSizeThreshold{10485760};
           if(section.size >= zipFileSizeThreshold) {
             off_t pa_offset = (fileOffset + section.offset) & ~(sysconf(_SC_PAGESIZE) - 1);
             size_t mappedLength = section.size + (fileOffset + section.offset) - pa_offset;
             return std::make_shared<RawBundleResources>(std::make_unique<MappedFile>(filePath, mappedLength, pa_offset));
           } else {
             void* zipData = malloc(section.size * sizeof(char));
             if (zipData) {
               std::unique_ptr<void, void(*)(void*)> scopedData(zipData, ::free);
               fs.seekg(fileOffset + section.offset);
               fs.read(reinterpret_cast<char*>(zipData), section.size);
               return std::make_shared<RawBundleResources>( std::make_unique<RawDataContainer>(std::move(scopedData), section.size) );
             }
           }
           
         }
         fs.seekg(lcmd_offset + sizeof(SegmentCommand) + ((i+1)*sizeof(Section)));
       }
    }
    return {};
  }

  std::shared_ptr<RawBundleResources> GetRawBundleResourceContainer() const override { return m_rawData; }

private:
  std::shared_ptr<RawBundleResources> m_rawData;
};

static std::vector<std::vector<uint32_t>> GetMachOIdents(std::ifstream& is)
{
  // magic (32 or 64 bit) | cputype | offset
  std::vector<std::vector<uint32_t>> idents;

  uint32_t magic;
  is.seekg(0);
  is.read(reinterpret_cast<char*>(&magic), sizeof magic);

  if (readBE(magic) == FAT_MAGIC) {
    is.seekg(0);
    fat_header fatHdr;
    is.read(reinterpret_cast<char*>(&fatHdr), sizeof fatHdr);
    std::unique_ptr<fat_arch[]> fatArchs(new fat_arch[readBE(fatHdr.nfat_arch)]);
    is.read(reinterpret_cast<char*>(fatArchs.get()),
            sizeof(fatArchs) * readBE(fatHdr.nfat_arch));
    const fat_arch* currArch = fatArchs.get();
    for (uint32_t i = 0; i < readBE(fatHdr.nfat_arch); ++i, ++currArch) {
      is.seekg(readBE(currArch->offset));
      mach_header machHdr;
      is.read(reinterpret_cast<char*>(&machHdr), sizeof machHdr);
      std::vector<uint32_t> ident(3, 0);
      ident[0] = machHdr.magic;
      ident[1] = readBE(currArch->cputype);
      ident[2] = readBE(currArch->offset);
      idents.push_back(ident);
    }
  } else {
    std::vector<uint32_t> ident(3, 0);
    ident[0] = magic;
    is.read(reinterpret_cast<char*>(&ident[1]), sizeof(uint32_t));
    idents.push_back(ident);
  }
  return idents;
}

// We can't get the identification bits for the currently
// running bundle (the host system) directly from the file
// because the file could itself be a fat binary with multiple
// possible target systems.
static std::vector<uint32_t> GetMachOIdent()
{
  // magic (32 or 64 bit) | cputype | offset
  std::vector<uint32_t> ident(3, 0);

#ifdef __LP64__
  ident[0] = MH_MAGIC_64;
#else
  ident[0] = MH_MAGIC;
#endif

#if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__)
  ident[1] = CPU_TYPE_POWERPC64;
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
  ident[1] = CPU_TYPE_POWERPC;
#elif defined(__sparc)
  ident[1] = CPU_TYPE_SPARC;
#elif defined(__x86_64__) || defined(_M_X64)
  ident[1] = CPU_TYPE_X86_64;
#elif defined(__i386) || defined(_M_IX86)
  ident[1] = CPU_TYPE_X86;
#elif defined(__arm__) || defined(_M_ARM)
  ident[1] = CPU_TYPE_ARM;
#endif

  return ident;
}

std::unique_ptr<BundleObjFile> CreateBundleMachOFile(const std::string& fileName)
{
  struct stat machStat;
  errno = 0;
  if (stat(fileName.c_str(), &machStat) != 0) {
    throw InvalidMachOException("Stat for " + fileName + " failed", errno);
  }

  std::size_t fileSize = machStat.st_size;

  // magic number
  if (fileSize < sizeof(uint32_t)) {
    throw InvalidMachOException("Missing magic number");
  }

  std::ifstream machFile(fileName.c_str(), std::ios_base::binary);
  machFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  std::vector<uint32_t> selfIdent = GetMachOIdent();
  std::vector<std::vector<uint32_t>> fileIdents = GetMachOIdents(machFile);

  std::vector<uint32_t> matchingIdent(3, 0);

  // check if the identifications match for the running application
  // and the Mach-O file (or one of the embedded files in the fat binary)
  for (std::size_t i = 0; i < fileIdents.size(); ++i) {
    if (memcmp(&fileIdents[i][0], &selfIdent[0], 2 * sizeof(uint32_t)) == 0) {
      matchingIdent = fileIdents[i];
    }
  }

  if (matchingIdent[0] == 0) {
    throw InvalidMachOException("Not a compatible Mach-O file or fat binary");
  }

  if (matchingIdent[0] == MH_MAGIC) {
    return std::make_unique<BundleMachOFile<MachO<MH_MAGIC>>>(machFile, matchingIdent[2], fileName);
  } else if (matchingIdent[0] == MH_MAGIC_64) {
    return std::make_unique<BundleMachOFile<MachO<MH_MAGIC_64>>>(machFile, matchingIdent[2], fileName);
  } else {
    throw InvalidMachOException(
      "Internal error: Mach-O magic field value is neither MH_MAGIC nor MH_MAGIC_64");
  }
}
}

#endif
