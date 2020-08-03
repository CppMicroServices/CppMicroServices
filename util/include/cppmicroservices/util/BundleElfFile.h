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

#if defined (US_PLATFORM_LINUX)

#include "BundleObjFile.h"
#include "MappedFile.h"

#include <cerrno>
#include <cstring>
#include <elf.h>
#include <fstream>
#include <map>
#include <memory>
#include <new>

#include <sys/stat.h>

namespace cppmicroservices {

struct InvalidElfException : public InvalidObjFileException
{
  InvalidElfException(std::string what, int errorNumber = 0)
    : InvalidObjFileException(std::move(what), errorNumber)
  {}
};

template<int>
struct Elf;

template<>
struct Elf<ELFCLASS32>
{
  typedef Elf32_Ehdr Ehdr;
  typedef Elf32_Shdr Shdr;
  typedef Elf32_Dyn Dyn;
  typedef Elf32_Addr Addr;
  typedef Elf32_Sym Sym;

  typedef Elf32_Half Half;
  typedef Elf32_Word Word;
  typedef Elf32_Off Off;

  static unsigned char GetSymbolEntryType(unsigned char info)
  {
    return ELF32_ST_TYPE(info);
  }
};

template<>
struct Elf<ELFCLASS64>
{
  typedef Elf64_Ehdr Ehdr;
  typedef Elf64_Shdr Shdr;
  typedef Elf64_Dyn Dyn;
  typedef Elf64_Addr Addr;
  typedef Elf64_Sym Sym;

  typedef Elf64_Half Half;
  typedef Elf64_Word Word;
  typedef Elf64_Off Off;

  static unsigned char GetSymbolEntryType(unsigned char info)
  {
    return ELF64_ST_TYPE(info);
  }
};

template<class ElfType>
class BundleElfFile
  : public BundleObjFile
  , private ElfType
{
public:
  typedef typename ElfType::Ehdr Ehdr;
  typedef typename ElfType::Shdr Shdr;
  typedef typename ElfType::Dyn Dyn;
  typedef typename ElfType::Addr Addr;
  typedef typename ElfType::Sym Sym;

  typedef typename ElfType::Half Half;
  typedef typename ElfType::Word Word;
  typedef typename ElfType::Off Off;

  BundleElfFile(std::ifstream& fs, std::size_t fileSize, const std::string& fileName)
    : m_rawData()
  {
    if (fileSize < sizeof(Ehdr)) {
      throw InvalidElfException("Missing ELF header");
    }

    fs.seekg(0);

    // Read the ELF header
    Ehdr elfHeader;
    fs.read(reinterpret_cast<char*>(&elfHeader), sizeof elfHeader);

    if (elfHeader.e_type != ET_DYN) {
      throw InvalidElfException("Not an ELF shared library");
    }

    if (elfHeader.e_shoff +
       (elfHeader.e_shnum * elfHeader.e_shentsize) >
       fileSize) {
      throw InvalidElfException("ELF section headers missing");
    }

    // read in all section headers
    auto sectionHeaders = std::make_unique<Shdr[]>(elfHeader.e_shnum);
    fs.seekg(elfHeader.e_shoff);
    fs.read(reinterpret_cast<char*>(sectionHeaders.get()),
      sizeof(Shdr) * elfHeader.e_shnum);

    // parse the .us_resources section
    // keep the buffer in a smart pointer to correctly clean up at end
    // of scope even if exceptions occur.
    auto buffer = std::make_unique<char[]>(sectionHeaders[elfHeader.e_shstrndx].sh_size);
    if (nullptr != buffer) {
      fs.seekg(sectionHeaders[elfHeader.e_shstrndx].sh_offset);
      fs.read(buffer.get(), sectionHeaders[elfHeader.e_shstrndx].sh_size);

      for (int i = 0; i < elfHeader.e_shnum; ++i) {
        if (0 == strcmp(".us_resources", (buffer.get() + sectionHeaders[i].sh_name))) {
          fs.seekg(sectionHeaders[i].sh_offset);
          auto zipContentSize = sectionHeaders[i].sh_size;
          if (0 < zipContentSize) {
            off_t pa_offset = (sectionHeaders[i].sh_offset) & ~(sysconf(_SC_PAGESIZE) - 1);
            size_t mappedLength = zipContentSize + (sectionHeaders[i].sh_offset) - pa_offset;
            m_rawData = std::make_shared<RawBundleResources>(std::make_unique<MappedFile>(fileName, mappedLength, pa_offset));
            break;
          }
        }
      }
    }
  }
  
  std::shared_ptr<RawBundleResources> GetRawBundleResourceContainer() const override  { return m_rawData; }

private:
  std::shared_ptr<RawBundleResources> m_rawData;
};

std::unique_ptr<BundleObjFile> CreateBundleElfFile(const std::string& fileName)
{
  struct stat elfStat;
  errno = 0;
  if (stat(fileName.c_str(), &elfStat) != 0) {
    throw InvalidElfException("Stat for " + fileName + " failed", errno);
  }

  std::size_t fileSize = elfStat.st_size;

  if (fileSize < EI_NIDENT) {
    throw InvalidElfException("Missing ELF identification");
  }

  std::ifstream elfFile(fileName.c_str(), std::ios_base::binary);
  elfFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  char elfIdent[EI_NIDENT];
  elfFile.read(elfIdent, sizeof elfIdent);

  if (memcmp(elfIdent, ELFMAG, SELFMAG) != 0) {
    throw InvalidElfException("Not an ELF object file");
  }

  std::ifstream selfFile(fileName, std::ios_base::binary);
  char selfIdent[EI_NIDENT];
  selfFile.read(selfIdent, sizeof selfIdent);
  selfFile.close();
  if (memcmp(elfIdent, selfIdent, EI_VERSION) != 0) {
    throw InvalidElfException("Not a compatible ELF object file");
  }

  if (elfIdent[EI_CLASS] == ELFCLASS32) {
    return std::unique_ptr<BundleObjFile>(new BundleElfFile<Elf<ELFCLASS32>>(elfFile, fileSize, fileName));
  } else if (elfIdent[EI_CLASS] == ELFCLASS64) {
    return std::unique_ptr<BundleObjFile>(new BundleElfFile<Elf<ELFCLASS64>>(elfFile, fileSize, fileName));
  } else {
    throw InvalidElfException("Unknown ELF format");
  }
}
}

#endif
