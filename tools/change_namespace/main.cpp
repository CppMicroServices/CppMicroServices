#include "ChangeNamespace.hpp"
#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    CLI::App app{
        "USAGE: change_namespace [options] <destination>\n"
        "Options:"
    };
    std::string cppms_src;
    std::string new_namespace;
    bool namespace_alias = false;
    std::string destination;
    
    // Custom validator to reject empty values and flag-like values
    auto non_empty_non_flag = [](std::string const& value) -> std::string
    {
        if (value.empty())
        {
            return "Value cannot be empty";
        }
        if (value[0] == '-')
        {
            return "Value cannot be a flag";
        }
        return "";
    };

    app.add_option("--cppms_src", cppms_src, "sets the location of the cppms tree to path. [REQUIRED]")
        ->required()
        ->type_name("PATH")
        ->check(non_empty_non_flag);
    
    app.add_option("--namespace", new_namespace, "rename the cppms namespace to name. [REQUIRED]")
        ->required()
        ->type_name("NAME")
        ->check(non_empty_non_flag);
    
    app.add_flag("--namespace-alias", namespace_alias, "makes namespace cppms an alias of the namespace set with --namespace.");
    app.set_help_flag("-h,--help", "Print usage and exit.");
    
    app.add_option("destination", destination, "Destination directory")
        ->required()
        ->type_name("DESTINATION")
        ->check(non_empty_non_flag);
        
    if(argc == 1) {
        std::cout <<"Just one argument"<<std::endl;
        std::cout << app.help() << std::endl;
        return 0;
    }
    
    try {
        app.parse(argc, argv);
    } catch(const CLI::CallForHelp&) {
        std::cout << app.help() << std::endl;
        return 0;
    } catch(const CLI::Error &e) {
        std::cerr << e.what() << std::endl;
        std::cerr << app.help() << std::endl;
        return EXIT_FAILURE;
    }
    
    // Set up and run your app
    ChangeNamespace cn_app;
    cn_app.set_cppms_src_path(cppms_src.c_str());
    cn_app.set_namespace(new_namespace.c_str());
    cn_app.set_namespace_alias(namespace_alias);
    cn_app.set_destination(destination.c_str());
    
    return cn_app.run();
}