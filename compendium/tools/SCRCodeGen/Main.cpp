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

#include <iostream>
#include <unordered_map>
#include <iomanip>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "json/json.h"
#include "ComponentInfo.hpp"
#include "ManifestParserFactory.hpp"
#include "ManifestParser.hpp"
#include "gtest/gtest_prod.h"
#include "ComponentInfo.hpp"
#include "Util.hpp"
#include "ComponentCallbackGenerator.hpp"
using codegen::ComponentCallbackGenerator;
using codegen::util::JsonValueValidator;
using codegen::util::ParseManifestOrThrow;
using codegen::util::WriteToFile;

int main(int argc, const char **argv, char**)
{
  int returnCode = 0;
  const int FailureReturnCode = -1;

  std::vector<std::string> args(argv, argv + argc);

  // Validate program options ordering.
  // Return the iterator corresponding to the option argument.
  // Error conditions:
  // 1. the option doesn't exist
  // 2. the option argument doesn't exist
  // 3. the option argument starts with a '-'
  auto findOrThrow = [&args](const std::string& key)
  {
    auto it = std::find(std::begin(args), std::end(args), key);
    if (it == args.end())
    {
      throw std::runtime_error("Cannot find option " + key);
    }
    ++it;
    if (it == args.end())
    {
      throw std::runtime_error("No argument provided for option " + key);
    }
    if (it->at(0) == '-')
    {
      throw std::runtime_error("The argument " + key + " cannot be an option i.e. it cannot start with -");
    }
    return it;
  };

  // Validate if file stream is open. Otherwise, throw
  auto checkFileOpenOrThrow = [](const auto& fstream)
  {
    if(!fstream.is_open())
    {
      throw std::runtime_error("Failed to open manifest file");
    }
  };

  try
  {
    auto it = findOrThrow("--manifest");
    std::string manifestFilePath = *it;
    it = findOrThrow("--out-file");
    std::string outFilePath= *it;
    // --include-headers is followed by 1 or more header strings. Parse until the end of
    // vector or until the next occurence of '-'
    std::vector<std::string> includeHeaderPaths;
    for (it = findOrThrow("--include-headers"); it != args.end() && !(it->at(0) == '-'); ++it)
    {
      includeHeaderPaths.push_back(*it);
    }

    std::ifstream manifestFile(manifestFilePath, std::ifstream::binary | std::ifstream::in);
    checkFileOpenOrThrow(manifestFile);
    const auto root = ParseManifestOrThrow(manifestFile);
    const auto scr =  JsonValueValidator(root, "scr", Json::ValueType::objectValue)();
    const auto version = JsonValueValidator(scr, "version", Json::ValueType::intValue)();
    const auto manifestParser = ManifestParserFactory::Create(version.asInt());
    const auto componentInfos = manifestParser->ParseAndGetComponentInfos(scr);
    ComponentCallbackGenerator compGen(includeHeaderPaths, componentInfos);
    WriteToFile(outFilePath, compGen.GetString());
  }
  catch (const std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
    returnCode = FailureReturnCode;
  }

  return returnCode;
}
