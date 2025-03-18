#include "ChangeNamespace.hpp"
#include "optionparser.h"

#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <vector>

// Custom argument class to handle non-empty arguments for option parser
struct Custom_Arg : public option::Arg
{
    static void
    printError(std::string const& msg1, option::Option const& opt, std::string const& msg2)
    {
        std::cerr << "ERROR: " << msg1 << opt.name << msg2 << std::endl;
    }

    static option::ArgStatus
    NonEmpty(option::Option const& option, bool msg)
    {
        if (option.arg != nullptr && option.arg[0] != '\0')
        {
            return option::ARG_OK;
        }
        if (msg)
        {
            printError("Option '", option, "' requires a non-empty argument\n");
        }
        return option::ARG_ILLEGAL;
    }
};

// Options supported by the tool
enum class OptionIndex
{
    UNKNOWN,
    HELP,
    CPPMS_SRC,
    NAMESPACE,
    NAMESPACE_ALIAS
};

// Specifying tool usage for the option parser
const option::Descriptor usage[] = {
    {        static_cast<int>(OptionIndex::UNKNOWN),
     0,      "",
     "",    option::Arg::None,
     "USAGE: change_namespace [options] <destination>\n\nOptions:"                                                                                                 },
    {           static_cast<int>(OptionIndex::HELP), 0,     "h",  "help",    option::Arg::None,                              "  -h, --help \tPrint usage and exit."},
    {      static_cast<int>(OptionIndex::CPPMS_SRC),
     0,      "",
     "cppms_src", Custom_Arg::NonEmpty,
     "  --cppms_src=<path> \tsets the location of the cppms tree to path. [REQUIRED]"                                                                              },
    {      static_cast<int>(OptionIndex::NAMESPACE),
     0,      "",
     "namespace", Custom_Arg::NonEmpty,
     "  --namespace=<name> \trename the cppms namespace to name. [REQUIRED]"                                                                                       },
    {static_cast<int>(OptionIndex::NAMESPACE_ALIAS),
     0,      "",
     "namespace-alias",    option::Arg::None,
     "  --namespace-alias \tmakes namespace cppms an alias of the namespace set with --namespace."                                                                 },
    {                                             0, 0, nullptr, nullptr,              nullptr,                                                             nullptr}
};

int
main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    argc -= (argc > 0);
    argv += (argc > 0); // Skip program name argv[0] if present

    // Setup option parser and parse the arugments provided
    option::Stats stats(static_cast<option::Descriptor const*>(usage), argc, argv);
    std::vector<option::Option> options(stats.options_max);
    std::vector<option::Option> buffer(stats.buffer_max);
    option::Parser parse(static_cast<option::Descriptor const*>(usage), argc, argv, &options[0], &buffer[0]);

    // Exit if error while parsing arguments
    if (parse.error())
    {
        return 1;
    }

    // Print usage if 'help' option is present or no arguments are provided
    if (options[static_cast<int>(OptionIndex::HELP)] || argc == 0)
    {
        option::printUsage(std::cout, static_cast<option::Descriptor const*>(usage));
        return 0;
    }

    /*for (option::Option* opt = options[static_cast<int>(OptionIndex::UNKNOWN)]; opt; opt = opt->next())
    {
        std::cout << "Unknown option: " << opt->name << "\n";
    }*/

    for (int i = 0; i < parse.nonOptionsCount(); ++i)
      std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << "\n";

    // Create application instance and set options
    ChangeNamespace cn_app;

    for (int i = 0; i < parse.optionsCount(); ++i)
    {
        option::Option& opt = buffer[i];
        switch (opt.index())
        {
            case static_cast<int>(OptionIndex::CPPMS_SRC):
                cn_app.set_cppms_src_path(opt.arg);
                break;
            case static_cast<int>(OptionIndex::NAMESPACE):
                cn_app.set_namespace(opt.arg);
                break;
            case static_cast<int>(OptionIndex::NAMESPACE_ALIAS):
                cn_app.set_namespace_alias(true);
                break;
        }
    }

    // Throw error if destination is not provided
    if (parse.nonOptionsCount() == 0)
    {
        std::cerr << "Error: Destination not provided.\n";
        option::printUsage(std::cerr, usage);
        return 1;
    }

    if (parse.nonOptionsCount() > 0)
    {
        cn_app.set_destination(parse.nonOption(parse.nonOptionsCount() - 1));
    }

    // Check if required options are provided
    if (!options[static_cast<int>(OptionIndex::CPPMS_SRC)] || !options[static_cast<int>(OptionIndex::NAMESPACE)])
    {
        std::cerr << "Error: Missing required options.\n";
        option::printUsage(std::cerr, usage);
        return 1;
    }

    // Run the application
    return cn_app.run();
}