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
#include "Util.hpp"

namespace codegen {
namespace util {

Json::Value ParseManifestOrThrow(std::istream& jsonStream)
{
  Json::Value root;
  Json::CharReaderBuilder rbuilder;
  rbuilder["rejectDupKeys"] = true;
  std::string errs;

  if (!Json::parseFromStream(rbuilder, jsonStream, &root, &errs))
  {
    throw std::runtime_error(errs);
  }
  return root;
}

void WriteToFile(const std::string& filePath, const std::string& content)
{
  auto fileStream = std::ofstream(filePath, std::ofstream::binary | std::ofstream::out);
  if(!fileStream.is_open())
  {
    throw std::runtime_error("Could not open out file at " + filePath);
  }
  fileStream << content;
  fileStream.close();
}
} // namespace util
} // namespace codegen
