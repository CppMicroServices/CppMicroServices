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

#if defined (US_PLATFORM_APPLE) || defined (US_PLATFORM_POSIX)

#ifndef CPPMICROSERVICES_MAPPEDFILE_H
#define CPPMICROSERVICES_MAPPEDFILE_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace cppmicroservices {

class MappedFile
{
public:
  MappedFile()
    : fileDesc(-1)
    , mappedAddress(nullptr)
    , mapSize(0) {}
  MappedFile(const std::string& fileLocation, size_t mapLength, off_t offset)
    : fileDesc(-1)
    , mappedAddress(nullptr)
    , mapSize(mapLength)
  {
    fileDesc = open(fileLocation.c_str(), O_RDONLY);
    if(fileDesc >= 0) {
      mappedAddress = mmap(0, mapLength, PROT_READ, MAP_PRIVATE, fileDesc, offset);
      if (MAP_FAILED == mappedAddress) {
        mappedAddress = nullptr;
        mapSize = 0;
      }
    }
  }
    
  ~MappedFile()
  {
    if(mappedAddress && mapSize) {
      munmap(mappedAddress, mapSize);
    }
    if(0 <= fileDesc) {
      close(fileDesc);
    }
  }

  MappedFile(const MappedFile&) = delete;
  MappedFile& operator=(const MappedFile&) = delete;
    
  size_t GetSize() const { return mapSize; }
  void* GetMappedAddress() const { return mappedAddress; }
private:
  int fileDesc;
  void* mappedAddress;
  size_t mapSize;
};

}
#endif // CPPMICROSERVICES_MAPPEDFILE_H

#endif // defined (US_PLATFORM_APPLE) || defined (US_PLATFORM_POSIX)
