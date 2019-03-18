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

#ifndef CPPMICROSERVICES_DATACONTAINER_H
#define CPPMICROSERVICES_DATACONTAINER_H

#include "cppmicroservices/GlobalConfig.h"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace cppmicroservices {

// A generic abstract base class to store
// and retrieve raw data bytes
class DataContainer
{
public:
  virtual ~DataContainer() = default;
  DataContainer(DataContainer&&) = default;
  DataContainer& operator=(DataContainer&&) = default;
  virtual void* GetData() const = 0;
  virtual std::size_t GetSize() const = 0;
protected:
  DataContainer() = default;
};

class RawDataContainer final : public DataContainer
{
public:
  RawDataContainer(std::unique_ptr<void, void(*)(void*)> data, std::size_t dataSize)
    : m_Data(std::move(data))
    , m_DataSize(dataSize)
    { }
  ~RawDataContainer() = default;
    
  void* GetData() const override { return m_Data.get(); }
  std::size_t GetSize() const override { return m_DataSize; }
  
private:
  std::unique_ptr<void, void(*)(void*)> m_Data;
  std::size_t m_DataSize;
};


}

#endif // CPPMICROSERVICES_BUNDLEOBJFILE_H
