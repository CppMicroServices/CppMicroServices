#include "optionparser.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/schema.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <iostream>

using namespace rapidjson;

/**
 * \brief Function to validate json file against a schema file
 *
 * \schema is the file path of the schema file
 * \jsonfile is the file path of the json data
 */
int validate(std::string schemafile, std::string jsonfile)
{
  // Read a JSON schema from file into Document
  Document d;
  char buffer[4096];

  {
    FILE *fp = fopen(schemafile.c_str(), "r");
    if (!fp) {
      std::cerr << "Schema file '" << schemafile << "' not found" << std::endl;
      return EXIT_FAILURE;
    }
    FileReadStream fs(fp, buffer, sizeof(buffer));
    if (d.ParseStream(fs).HasParseError()) {
      std::cerr << "Schema file '" << schemafile << "' is not a valid JSON" << std::endl;
      std::cerr << "Error(offset " << static_cast<unsigned>(d.GetErrorOffset()) << "): " << GetParseError_En(d.GetParseError()) << std::endl;
      fclose(fp);
      return EXIT_FAILURE;
    }
    fclose(fp);
  }

  // Then convert the Document into SchemaDocument
  SchemaDocument sd(d);
  SchemaValidator validator(sd);
  Document json;
  {
    FILE *fp = fopen(jsonfile.c_str(), "r");
    if (!fp) {
      std::cout << "Data file '" << jsonfile << "' not found" << std::endl;
      return EXIT_FAILURE;
    }
    FileReadStream is(fp, buffer, sizeof(buffer));
    if (json.ParseStream(is).HasParseError()) {
      // the input is not a valid JSON.
      std::cerr << "Input data file " << jsonfile << " is not a valid JSON file" << std::endl;
      std::cerr <<"Error(offset " << static_cast<unsigned>(json.GetErrorOffset()) << "): " << GetParseError_En(json.GetParseError()) << std::endl;
      fclose(fp);
      return EXIT_FAILURE;
    }
    fclose(fp);
  }

  if (!json.Accept(validator)) {
    std::cout << "JSON file " << jsonfile << " does not match schema file " << schemafile << std::endl;
    StringBuffer sb;
    PrettyWriter<StringBuffer> w(sb);
    validator.GetError().Accept(w);
    std::cerr << "Error report:" << std::endl << sb.GetString() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

struct Custom_Arg : public option::Arg
{
  static option::ArgStatus NonEmpty(const option::Option& option, bool msg)
  {
    auto retVal = option::ARG_OK;
    if (option.arg == 0 || option.arg[0] == 0) {
      if (msg) {
        std::cerr << "ERROR: Option '" << option.name << "' requires a non-empty argument" << std::endl;
      }
      retVal = option::ARG_ILLEGAL;
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

int main(int argc, char *argv[]) {
  argc -= (argc > 0);
  argv += (argc > 0); // skip program name argv[0]
  option::Stats stats(usage, argc, argv);
  std::unique_ptr<option::Option[]> options(
                                            new option::Option[stats.options_max]);
  std::unique_ptr<option::Option[]> buffer(
                                           new option::Option[stats.buffer_max]);
  option::Parser parse(true, usage, argc, argv, options.get(), buffer.get());

  if (argc == 0 || options[HELP]) {
    option::printUsage(std::clog, usage);
    return EXIT_SUCCESS;
  }

  if (!options[SCHEMAFILE]) {
    std::cerr << "A schema file must be specified using the --schema-file option. Check usage." << std::endl;
    return EXIT_FAILURE;
  }

  if (!options[JSONFILE]) {
    std::cerr << "A json file must be specified using the --json-file option. Check usage." << std::endl;
    return EXIT_FAILURE;
  }

  std::string jsonfilepath(options[JSONFILE].arg);
  std::string schemafilepath(options[SCHEMAFILE].arg);
  return validate(schemafilepath, jsonfilepath);
}
