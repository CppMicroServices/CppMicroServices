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

#include "BundleObjFile.h"

#include "cppmicroservices_elf.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <map>

#include <sys/stat.h>

namespace cppmicroservices {

struct InvalidElfException : public InvalidObjFileException
{
  InvalidElfException(const std::string& what, int errorNumber = 0)
    : InvalidObjFileException(what, errorNumber)
  {}
};

template<int>
struct Elf;

template<>
struct Elf<ELFCLASS32>
{
  using Ehdr = Elf32_Ehdr;
  using Shdr = Elf32_Shdr;
  using Dyn = Elf32_Dyn;
  using Addr = Elf32_Addr;
  using Sym = Elf32_Sym;

  using Half = Elf32_Half;
  using Word = Elf32_Word;
  using Off = Elf32_Off;

  static unsigned char GetSymbolEntryType(unsigned char info)
  {
    return ELF32_ST_TYPE(info);
  }
};

template<>
struct Elf<ELFCLASS64>
{
  using Ehdr = Elf64_Ehdr;
  using Shdr = Elf64_Shdr;
  using Dyn = Elf64_Dyn;
  using Addr = Elf64_Addr;
  using Sym = Elf64_Sym;

  using Half = Elf64_Half;
  using Word = Elf64_Word;
  using Off = Elf64_Off;

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
  using Ehdr = typename ElfType::Ehdr;
  using Shdr = typename Elf<1>::Shdr;
  using Dyn = typename Elf<2>::Dyn;
  using Addr = typename ElfType::Addr;
  using Sym = typename Elf<2>::Sym;

  using Half = typename Elf<1>::Half;
  using Word = typename Elf<2>::Word;
  using Off = typename Elf<2>::Off;

  BundleElfFile(std::ifstream& fs, std::size_t fileSize)
    : m_SectionHeaders(nullptr)
  {
    if (fileSize < sizeof(Ehdr)) {
      throw InvalidElfException("Missing ELF header");
    }

    fs.seekg(0);

    // Read the ELF header
    fs.read(reinterpret_cast<char*>(&m_FileHeader), sizeof m_FileHeader);

    if (m_FileHeader.e_type != ET_DYN) {
      throw InvalidElfException("Not an ELF shared library");
    }

    if (m_FileHeader.e_shoff +
          (m_FileHeader.e_shnum * m_FileHeader.e_shentsize) >
        fileSize) {
      throw InvalidElfException("ELF section headers missing");
    }

    // read in all section headers
    m_SectionHeaders = new Shdr[m_FileHeader.e_shnum];
    fs.seekg(m_FileHeader.e_shoff);
    fs.read(reinterpret_cast<char*>(m_SectionHeaders),
            sizeof *m_SectionHeaders * m_FileHeader.e_shnum);

    // parse the .dynamic section
    Shdr* dynamicHdr = this->FindSectionHeader(SHT_DYNAMIC);
    if (dynamicHdr == nullptr) {
      throw InvalidElfException("ELF .dynamic section header missing");
    }
    char* strTab = this->GetStringTable(fs, dynamicHdr);
    Dyn dynamicSecEntry;
    fs.seekg(dynamicHdr->sh_offset);
    fs.read(reinterpret_cast<char*>(&dynamicSecEntry), sizeof dynamicSecEntry);
    while (dynamicSecEntry.d_tag != DT_NULL) {
      if (dynamicSecEntry.d_tag == DT_SONAME) {
        m_Soname = strTab + dynamicSecEntry.d_un.d_val;
      } else if (dynamicSecEntry.d_tag == DT_NEEDED) {
        m_Needed.push_back(strTab + dynamicSecEntry.d_un.d_val);
      }
      fs.read(reinterpret_cast<char*>(&dynamicSecEntry),
              sizeof dynamicSecEntry);
    }

    // parse the .dynsym section
    Shdr* dynsymHdr = this->FindSectionHeader(SHT_DYNSYM);
    if (dynsymHdr == nullptr) {
      throw InvalidElfException("ELF .dynsym section header missing");
    }
    strTab = this->GetStringTable(fs, dynsymHdr);
    auto sectionSize = static_cast<std::size_t>(dynsymHdr->sh_size);
    auto* symbols = new char[sectionSize];
    fs.seekg(dynsymHdr->sh_offset);
    fs.read(symbols, sectionSize);
    auto* symEntry = reinterpret_cast<Sym*>(symbols);
    for (std::size_t symIndex = 0;
         symIndex < sectionSize / dynsymHdr->sh_entsize;
         ++symIndex, ++symEntry) {
      if (symEntry->st_shndx == SHN_UNDEF ||
          this->GetSymbolEntryType(symEntry->st_info) != STT_FUNC) {
        continue;
      }
      std::string symName = strTab + symEntry->st_name;
      if (this->ExtractBundleName(symName, m_BundleName)) {
        break;
      }
    }
    delete[] symbols;

    for (auto
           iter = m_StrTblIndexToStrArray.begin(),
           iterEnd = m_StrTblIndexToStrArray.end();
         iter != iterEnd;
         ++iter) {
      delete[] iter->second;
    }

    delete[] m_SectionHeaders;
  }

  std::vector<std::string> GetDependencies() const override { return m_Needed; }

  std::string GetLibName() const override { return m_Soname; }

  std::string GetBundleName() const override { return m_BundleName; }

private:
  Ehdr m_FileHeader;
  Shdr* m_SectionHeaders;

  typedef std::map<Word, char*> StrTblMapType;
  StrTblMapType m_StrTblIndexToStrArray;

  std::vector<std::string> m_Needed;
  std::string m_Soname;
  std::string m_BundleName;

  Shdr* FindSectionHeader(Word type, Half startIndex = 0) const
  {
    Shdr* shdr = m_SectionHeaders + startIndex;
    for (int i = startIndex; i < m_FileHeader.e_shnum; ++i, ++shdr) {
      if (shdr->sh_type == type) {
        return shdr;
      }
    }
    return nullptr;
  }

  char* GetStringTable(std::ifstream& fs, Shdr* shdr)
  {
    if (shdr->sh_type != SHT_DYNAMIC && shdr->sh_type != SHT_SYMTAB &&
        shdr->sh_type != SHT_DYNSYM) {
      return nullptr;
    }

    Word strTblHdrIdx = shdr->sh_link;
    auto iter =
      m_StrTblIndexToStrArray.find(strTblHdrIdx);
    if (iter != m_StrTblIndexToStrArray.end()) {
      return iter->second;
    }

    Shdr* strTblHdr = m_SectionHeaders + strTblHdrIdx;

    fs.seekg(strTblHdr->sh_offset);
    auto* strTbl = new char[static_cast<std::size_t>(strTblHdr->sh_size)];
    fs.read(strTbl, strTblHdr->sh_size);
    m_StrTblIndexToStrArray.insert(std::make_pair(strTblHdrIdx, strTbl));

    return strTbl;
  }
};

BundleObjFile* CreateBundleElfFile(const char* selfName,
                                   const std::string& fileName)
{
  struct stat elfStat;
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

  std::ifstream selfFile(selfName, std::ios_base::binary);
  char selfIdent[EI_NIDENT];
  selfFile.read(selfIdent, sizeof selfIdent);
  selfFile.close();
  if (memcmp(elfIdent, selfIdent, EI_VERSION) != 0) {
    throw InvalidElfException("Not a compatible ELF object file");
  }

  if (elfIdent[EI_CLASS] == ELFCLASS32) {
    return new BundleElfFile<Elf<ELFCLASS32>>(elfFile, fileSize);
  } else if (elfIdent[EI_CLASS] == ELFCLASS64) {
    return new BundleElfFile<Elf<ELFCLASS64>>(elfFile, fileSize);
  } else {
    throw InvalidElfException("Wrong ELF class");
  }
}
}
