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

#include <fstream>   // std::ifstream
#include <iostream>  // std::cerr
#include <memory>    // std::unique_ptr
#include <sstream>   // std::sstream
#include <stdexcept> // std::runtime_error
#include <string>    // std::to_string

#include "CLI/CLI11.hpp"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h" // depends on ifstream included above
#include "rapidjson/prettywriter.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"

/**
 * \brief Utility function to create a rapidjson::Document object from a file path
 *
 * \jsonfile is the file path of the json data
 *
 * \returns rapidjson::Document object representing the json file
 * \throws std::runtime_error if the input file is non-existent or is not a valid json file
 */
rapidjson::Document
ReadJSONDocument(std::string const& filePath)
{
    std::ifstream fileStream(filePath.c_str());
    if (!fileStream.is_open())
    {
        throw std::runtime_error("Failed to open file at " + filePath);
    }
    rapidjson::IStreamWrapper streamWrapper(fileStream);
    rapidjson::Document doc;
    doc.ParseStream(streamWrapper);
    if (doc.HasParseError())
    {
        throw std::runtime_error("File '" + filePath + "' is not a valid JSON\n Error(offset "
                                 + std::to_string(doc.GetErrorOffset())
                                 + "): " + GetParseError_En(doc.GetParseError()));
    }
    return doc;
}

/**
 * \brief Function to validate json file against a schema file
 *
 * \schemafile is the file path of the schema
 * \jsonfile is the file path of the json data
 *
 * \returns std::pair<bool, std::string>. The first element of the pair is true
 *          if the json file matches the schema, error otherwise. In case of
 *          error, the second element contains the error report
 */
std::pair<bool, std::string>
validate(std::string const& jsonfile, std::string const& schemafile) noexcept
{
    try
    {
        rapidjson::SchemaDocument sd(ReadJSONDocument(schemafile));
        rapidjson::SchemaValidator validator(sd);
        rapidjson::Document json = ReadJSONDocument(jsonfile);
        if (!json.Accept(validator))
        {
            rapidjson::StringBuffer sb;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
            validator.GetError().Accept(w);
            std::stringstream errStream;
            errStream << "JSON file " << jsonfile << " does not match schema file " << schemafile << std::endl;
            errStream << "Error report:" << std::endl;
            errStream << sb.GetString() << std::endl;
            throw std::runtime_error(errStream.str());
        }
    }
    catch (std::exception const& e)
    {
        return std::make_pair(false, e.what());
    }
    return std::make_pair(true, "JSON file" + jsonfile + " matches the schema " + schemafile);
}

int
main(int argc, char* argv[])
{
    CLI::App app{"jsonschemavalidator: Validate a JSON file against a JSON schema."};

    std::string schemafilepath;
    std::string jsonfilepath;

    app.add_option("-s,--schema-file", schemafilepath, "Schema file path")
        ->required()
        ->check(CLI::ExistingFile)
        ->type_name("SCHEMAFILE");

    app.add_option("-j,--json-file", jsonfilepath, "JSON file to be validated")
        ->required()
        ->check(CLI::ExistingFile)
        ->type_name("JSONFILE");

    app.set_help_flag("-h,--help", "Print usage and exit.");

    // Custom usage message
    app.footer("\nExample:\n  jsonschemavalidator -s myschema.json -j mydata.json\n");

    int retVal = EXIT_SUCCESS;
    
    // If no arguments (argc==1) or only --help/-h, print usage and exit 0
    bool only_help = false;
    for(int i=1; i<argc; ++i) {
        std::string arg(argv[i]);
        if(arg == "-h" || arg == "--help") {
            only_help = true;
            break;
        }
    }
    if(argc == 1 || only_help) {
        std::cout << app.help() << std::endl;
        return EXIT_SUCCESS;
    }

    try {
        app.parse(argc, argv);
        auto result = validate(jsonfilepath, schemafilepath);
        if (!result.first)
        {
            std::cerr << result.second << std::endl;
            retVal = EXIT_FAILURE;
        }
    }
    catch(const CLI::ParseError &e) {
        // CLI11 will print the error and usage
        return app.exit(e);
    }

    return retVal;
}
