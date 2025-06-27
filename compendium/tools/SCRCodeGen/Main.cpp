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

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdarg>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "ComponentInfo.hpp"
#include "ManifestParser.hpp"
#include "ManifestParserFactory.hpp"
#include "json/json.h"
#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "ComponentCallbackGenerator.hpp"
#include "ComponentInfo.hpp"
#include "Util.hpp"
using codegen::ComponentCallbackGenerator;
using codegen::util::JsonValueValidator;
using codegen::util::ParseManifestOrThrow;
using codegen::util::WriteToFile;

int
main(int argc, char const** argv, char**)
{
    int returnCode = 0;
    int const FailureReturnCode = -1;

    try
    {
        std::vector<std::string> args(argv, argv + argc);

    	// Validate program options ordering.
	// Return the iterator corresponding to the option argument.
    	// Error conditions:
    	// 1. the option doesn't exist
    	// 2. the option argument doesn't exist
    	// 3. the option argument starts with a '-'
    	auto findOrThrow = [&args](std::string const& key)
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
    	auto checkFileOpenOrThrow = [](auto const& fstream)
    	{
            if (!fstream.is_open())
            {
                throw std::runtime_error("Failed to open manifest file");
            }
    	};

        auto it = findOrThrow("--manifest");
        std::string manifestFilePath = *it;
        it = findOrThrow("--out-file");
        std::string outFilePath = *it;
        // --include-headers is followed by 1 or more header strings. Parse until the end of
        // vector or until the next occurence of '-'
        std::vector<std::string> includeHeaderPaths;
        for (it = findOrThrow("--include-headers"); it != args.end() && !(it->at(0) == '-'); ++it)
        {
            includeHeaderPaths.push_back(*it);
        }

        std::ifstream manifestFile(manifestFilePath, std::ifstream::binary | std::ifstream::in);
        checkFileOpenOrThrow(manifestFile);
        auto const root = ParseManifestOrThrow(manifestFile);
        auto const scr = JsonValueValidator(root, "scr", Json::ValueType::objectValue)();
        auto const version = JsonValueValidator(scr, "version", Json::ValueType::intValue)();
        auto const manifestParser = ManifestParserFactory::Create(version.asInt());
        auto const componentInfos = manifestParser->ParseAndGetComponentInfos(scr);
        ComponentCallbackGenerator compGen(includeHeaderPaths, componentInfos);
        WriteToFile(outFilePath, compGen.GetString());
    }
    catch (std::exception const& ex)
    {
        std::cerr << ex.what() << std::endl;
        returnCode = FailureReturnCode;
    }

    return returnCode;
}
