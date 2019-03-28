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

#include "optionparser.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <iostream>

using namespace rapidjson;

// functor object for deleting FILE objects
struct FILEDeleter
{
  typedef FILE* pointer;
  void operator()(FILE *pFile)
  {
    if (pFile)
      fclose(pFile);
  }
};

/**
 * \brief Utility function to create a rapidjson::Document object from a file path
 *
 * \jsonfile is the file path of the json data
 *
 * \returns rapidjson::Document object representing the json file
 * \throws std::runtime_error if the input file is non-existent or is not a valid json file
 */
Document ReadJSONDocument(const std::string& filePath)
{
  std::unique_ptr<FILE, FILEDeleter> fp(fopen(filePath.c_str(), "r"));
  if (!fp)
  {
    std::string err("File '" + filePath + "' not found");
    throw std::runtime_error(err);
  }
  char buffer[4096];
  FileReadStream fs(fp.get(), buffer, sizeof(buffer));
  Document doc;
  doc.ParseStream(fs);
  if(doc.HasParseError())
  {
    throw std::runtime_error("File '" + filePath + "' is not a valid JSON\n Error(offset " + std::to_string(doc.GetErrorOffset()) + "): " + GetParseError_En(doc.GetParseError()));
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
std::pair<bool, std::string> validate(const std::string& jsonfile, const std::string& schemafile) noexcept
{
  try
  {
    SchemaDocument sd(ReadJSONDocument(schemafile));
    SchemaValidator validator(sd);
    Document json = ReadJSONDocument(jsonfile);
    if (!json.Accept(validator))
    {
      std::string errorMsg("JSON file " + jsonfile + " does not match schema file " + schemafile);
      StringBuffer sb;
      PrettyWriter<StringBuffer> w(sb);
      validator.GetError().Accept(w);
      errorMsg.append("\nError report:\n");
      errorMsg.append(sb.GetString());
      throw std::runtime_error(errorMsg);
    }
  }
  catch(std::exception& e)
  {
    return std::make_pair(false, e.what());
  }
  return std::make_pair(true, "JSON file" + jsonfile + " matches the schema " + schemafile);
}

/**
 * Subclass of option::Arg used to validate input options for the program
 */
struct Custom_Arg : public option::Arg
{
  /**
   * This callback is used to error out if the option is empty
   */
  static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
  {
    auto retVal = option::ARG_OK;
    if (option.arg == 0 || option.arg[0] == 0)
    {
      retVal = option::ARG_ILLEGAL;
      if (msg)
      {
        std::cerr << "ERROR: Option '" << option.name << "' requires a non-empty argument" << std::endl;
      }
    }
    return retVal;
  }
};

enum OptionIndex
{
  UNKNOWN,
  HELP,
  SCHEMAFILE,
  JSONFILE
};

const option::Descriptor usage[] = {
  { UNKNOWN,
    0,
    "",
    "",
    Custom_Arg::None,
    "\nUSAGE: jsonschemavalidator [options]\n\n"
    "Options:" },
  { HELP,
    0,
    "h",
    "help",
    Custom_Arg::None,
    " --help, -h  \tPrint usage and exit." },
  { SCHEMAFILE,
    0,
    "s",
    "schema-file",
    Custom_Arg::NonEmpty,
    " --schema-file, -s \tSchema file path" },
  { JSONFILE,
    0,
    "j",
    "json-file",
    Custom_Arg::NonEmpty,
    " --json-file, -j \tJSON file to be validated" },
  { 0, 0, 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
  argc -= (argc > 0);
  argv += (argc > 0); // skip program name in argv[0]
  option::Stats stats(usage, argc, argv);
  option::Option options[stats.options_max], buffer[stats.buffer_max];
  option::Parser parse(true, usage, argc, argv, options, buffer);
  auto retVal = EXIT_SUCCESS;
  if (argc == 0 || options[HELP])
  {
    option::printUsage(std::clog, usage);
  }
  else if (!options[SCHEMAFILE])
  {
    std::cerr << "A schema file must be specified using the --schema-file(-s) option. Check usage." << std::endl;
    retVal = EXIT_FAILURE;
  }
  else if (!options[JSONFILE])
  {
    std::cerr << "A json file must be specified using the --json-file(-j) option. Check usage." << std::endl;
    retVal = EXIT_FAILURE;
  }
  else
  {
    std::string jsonfilepath(options[JSONFILE].arg);
    std::string schemafilepath(options[SCHEMAFILE].arg);
    auto result = validate(jsonfilepath, schemafilepath);
    if(!result.first)
    {
      std::cerr << result.second << std::endl;
      retVal = EXIT_FAILURE;
    }
  }
  return retVal;
}
