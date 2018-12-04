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

#ifndef CPPMICROSERVICES_BUNDLEOBJFILE_H
#define CPPMICROSERVICES_BUNDLEOBJFILE_H

#include "cppmicroservices/GlobalConfig.h"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace cppmicroservices {

struct InvalidObjFileException : public std::exception
{
  ~InvalidObjFileException() throw() {}
  InvalidObjFileException(std::string  what, int errorNumber = 0);

  virtual const char* what() const throw();
private:
  std::string m_What;
};

// Represents the raw bundle resource data.
// The data buffer contains the bits representing a zip file.
// The data size is the zip file's size in bytes.
class RawBundleResources
{
public:
  RawBundleResources(void* data, std::size_t dataSize)
  : m_Data(data)
  , m_DataSize(dataSize)
  { }

  operator bool() const
  {
    return m_Data && (m_DataSize > 0);
  }

  void* GetData() const { return m_Data; }
  std::size_t GetSize() const { return m_DataSize; }
    
private:
  void* m_Data;
  std::size_t m_DataSize;
};

class BundleObjFile
{
public:

  virtual ~BundleObjFile() {}

  /// Return a vector of linked libraries which this bundle depends on.
  virtual std::vector<std::string> GetDependencies() const = 0;
  /// Return the this bundle's filename
  virtual std::string GetLibraryName() const = 0;
  /// Return the raw bundle resource container bits.
  virtual std::shared_ptr<RawBundleResources> GetRawBundleResourceContainer() const = 0;
};

}

#endif // CPPMICROSERVICES_BUNDLEOBJFILE_H
