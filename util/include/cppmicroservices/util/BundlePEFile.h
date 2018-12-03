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

#if defined (US_PLATFORM_WINDOWS)

#include "BundleObjFile.h"

#include "cppmicroservices_pe.h"

#include <cerrno>
#include <fstream>
#include <memory>

#include <sys/stat.h>

#include <Windows.h>

namespace cppmicroservices {

struct hModuleDeleter
{
  // if the unique_ptr's deleter contains a nested type named 'pointer',
  // then the unique_ptr will use that type for its managed object
  // pointer instead of T*.
  typedef HMODULE pointer;

  void operator()(HMODULE handle) const {
    if (handle) {
      FreeLibrary(handle);
    }
  }
};

struct InvalidPEException : public InvalidObjFileException
{
  InvalidPEException(const std::string& what, int errorNumber = 0)
    : InvalidObjFileException(what, errorNumber)
  {}
};

template<int> struct PE;

template<> struct PE<OPTIONAL_PE32_MAGIC>
{
  typedef OptionalHeader32 Ohdr;
};

template<> struct PE<OPTIONAL_PE64_MAGIC>
{
  typedef OptionalHeader64 Ohdr;
};

template<class PEType>
class BundlePEFile : public BundleObjFile, private PEType
{
public:

  typedef typename PEType::Ohdr Ohdr;

  BundlePEFile(std::ifstream& fs, std::string location, const COFFHeader& coffHeader)
    : m_CoffHeader(coffHeader)
    , m_SectionHeaders(nullptr)
    , m_Location(std::move(location))
    , m_LoadLibraryHandle(nullptr)
    , m_rawData(nullptr)
  {
    HMODULE hBundleResources = LoadLibraryEx(m_Location.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    // RAII - automatically free the library handle on object destruction
    m_LoadLibraryHandle = std::unique_ptr<HMODULE, hModuleDeleter>(hBundleResources, hModuleDeleter{});
    if(hBundleResources) {
      HRSRC hResource = FindResource(hBundleResources, "US_RESOURCES", MAKEINTRESOURCE(300));
      HGLOBAL hRes = LoadResource(hBundleResources, hResource);
      const LPVOID res = LockResource(hRes);
      const DWORD zipSizeInBytes = SizeofResource(hBundleResources, hResource);
      if (0 < zipSizeInBytes) {
        m_rawData = std::make_shared<RawBundleResources>(res, zipSizeInBytes);
      }
    }

    fs.read(reinterpret_cast<char*>(&m_OptionalHeader), sizeof m_OptionalHeader);

    m_SectionHeaders = std::unique_ptr<SectionHeader[]>(new SectionHeader[coffHeader.NumberOfSections]);
    fs.read(reinterpret_cast<char*>(m_SectionHeaders.get()), sizeof(SectionHeader) * coffHeader.NumberOfSections);

    // check if there are any exports
    if (m_OptionalHeader.DataDirectories[IMAGE_DIRECTORY_ENTRY_EXPORT].Size > 0) {
      // find the export section
      std::size_t exportSectionOffset = GetOffset(m_OptionalHeader.DataDirectories[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
      fs.seekg(exportSectionOffset);
      ExportDirectory exportDir;
      fs.read(reinterpret_cast<char*>(&exportDir), sizeof exportDir);

      // get the library name
      fs.seekg(GetOffset(exportDir.Name));
      std::getline(fs, m_Soname, '\0');
    }

    // check if there are any imports
    if (m_OptionalHeader.DataDirectories[IMAGE_DIRECTORY_ENTRY_IMPORT].Size > 0) {
      // find the import section
      SectionHeader* importSecHeader = GetSectionHeader(IMAGE_DIRECTORY_ENTRY_IMPORT);
      if (importSecHeader) {
        std::size_t importSectionOffset = GetOffset(m_OptionalHeader.DataDirectories[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        fs.seekg(importSectionOffset);
        IMAGE_IMPORT_DESCRIPTOR importDesc;
        fs.read(reinterpret_cast<char*>(&importDesc), sizeof importDesc);
        int currDirEntry = 1;
        while (importDesc.Name != 0) {
          fs.seekg(GetOffset(importDesc.Name));
          std::string name;
          std::getline(fs, name, '\0');
          m_Needed.push_back(name);
              
          fs.seekg(importSectionOffset + (currDirEntry * sizeof importDesc));
          fs.read(reinterpret_cast<char*>(&importDesc), sizeof importDesc);
          ++currDirEntry;
        }
      }
    }
  }

  std::vector<std::string> GetDependencies() const override { return m_Needed; }

  std::string GetLibraryName() const override { return m_Soname; }
    
  std::shared_ptr<RawBundleResources> GetRawBundleResourceContainer() const override
  {
    return m_rawData;
  }

private:

  std::size_t GetOffset(uint32_t rva)
  {
    SectionHeader* psh = this->FindSectionHeader(rva);
    if (psh == nullptr) {
      throw InvalidPEException("Invalid relative virtual address");
    }
    return psh->PointerToRawData + (rva - psh->VirtualAddress);
  }

  SectionHeader* FindSectionHeader(uint32_t rva)
  {
    SectionHeader* psh = m_SectionHeaders.get();

    // Locate section containing rva
    int i = 0;
    while (i++ < m_CoffHeader.NumberOfSections) {
      if (psh->VirtualAddress <= rva &&
          psh->VirtualAddress + psh->SizeOfRawData > rva) {
        break;
      }
      psh++;
    }
    if (i > m_CoffHeader.NumberOfSections) {
      return nullptr;
    }
    return psh;
  }

  SectionHeader* GetSectionHeader(std::size_t dataDirIndex)
  {
    if (dataDirIndex >= m_OptionalHeader.NumberOfRvaAndSizes) {
      return nullptr;
    }

    uint32_t dataDirVA = m_OptionalHeader.DataDirectories[dataDirIndex].VirtualAddress;
    return FindSectionHeader(dataDirVA);
  }

  Ohdr m_OptionalHeader;

  const COFFHeader& m_CoffHeader;
  std::unique_ptr<SectionHeader[]> m_SectionHeaders;

  std::vector<std::string> m_Needed;
  std::string m_Soname;
  const std::string m_Location;
  std::unique_ptr<HMODULE, hModuleDeleter> m_LoadLibraryHandle;
  std::shared_ptr<RawBundleResources> m_rawData;
};

std::unique_ptr<BundleObjFile> CreateBundlePEFile(const std::string& fileName)
{
  struct stat peStat;
  errno = 0;
  if (stat(fileName.c_str(), &peStat) != 0) {
    throw InvalidPEException("Stat for " + fileName + " failed", errno);
  }

  std::size_t fileSize = peStat.st_size;

  if (fileSize < sizeof(DOSHeader)) {
    throw InvalidPEException("Missing DOS header");
  }

  std::ifstream peFile(fileName.c_str(), std::ios_base::binary);
  peFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  DOSHeader dosHdr;
  peFile.read(reinterpret_cast<char*>(&dosHdr), sizeof(DOSHeader));

  if (dosHdr.Magic != DOS_SIGNATURE) {
    throw InvalidPEException("Not a DOS executable file");
  }

  if (dosHdr.AddressOfNewExeHeader == 0) {
    throw InvalidPEException("Not a valid PE file");
  }

  peFile.seekg(dosHdr.AddressOfNewExeHeader);
  uint32_t peSignature;
  peFile.read(reinterpret_cast<char*>(&peSignature), sizeof peSignature);
  if (peSignature != PE_SIGNATURE) {
    throw InvalidPEException("Not a valid PE file");
  }

  if (fileSize < dosHdr.AddressOfNewExeHeader + 4 + sizeof(COFFHeader)) {
    throw InvalidPEException("Missing COFF File Header");
  }

  COFFHeader coffHeader;
  peFile.read(reinterpret_cast<char*>(&coffHeader), sizeof coffHeader);

  // TODO
  // compare coffHeader.Machine with running type

  if ((coffHeader.Characteristics & IMAGE_FILE_DLL) == 0) {
    throw InvalidPEException("Not a dynamic-link library (DLL)");
  }

  if (coffHeader.SizeOfOptionalHeader == 0) {
    throw InvalidPEException("Missing optional PE header");
  }

  uint16_t stateMagic = 0;
  peFile.read(reinterpret_cast<char*>(&stateMagic), sizeof stateMagic);
  peFile.seekg(- static_cast<int>(sizeof stateMagic), std::ios_base::cur);

  if (stateMagic == OPTIONAL_PE32_MAGIC) {
    return std::unique_ptr<BundleObjFile>(new BundlePEFile<PE<OPTIONAL_PE32_MAGIC>>(peFile, fileName, coffHeader));
  } else if (stateMagic == OPTIONAL_PE64_MAGIC) {
    return std::unique_ptr<BundleObjFile>(new BundlePEFile<PE<OPTIONAL_PE64_MAGIC>>(peFile, fileName, coffHeader));
  } else {
    throw InvalidPEException("Unknown PE format");
  }

}

}

#endif
