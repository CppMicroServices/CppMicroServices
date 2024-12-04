#include "ChangeNamespace.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <list>
#include <string>

void print_usage_info()
{
   std::cout <<
      "Usage:\n"
      "   change_namespace [options] output-path\n"
      "\n"
      "Options:\n"
      "   --cppms=path      sets the location of the cppms tree to path\n"
      "   --namespace=name  rename the cppms namespace to name.\n"
      "   --namespace-alias makes namespace cppms an alias of the namespace set with --namespace.\n"
      "\n"
      "output-path:         the path to which files will be copied\n"
      "   --help/-h         show usage help\n";
}

int main(int argc, char* argv[])
{
   // If no arguments are passed, print usage info
   if(argc < 2)
   {
      std::cout << "Error: insufficient arguments." << std::endl;
      print_usage_info();
      return 1;
   }

   // Create application object
   cn_app_ptr app_ptr(CNApplication::create());
   
   // Parse through the arguments to determine the operation to be performed
   std::list<const char*> positional_args;
   for(int i = 1; i < argc; ++i)
   {
      if(0 == std::strcmp("-h", argv[i])
         || 0 == std::strcmp("--help", argv[i]))
      {
         print_usage_info();
         return 0;
      }  
      else if(0 == std::strncmp("--cppms=", argv[i], 8))
      {
         app_ptr->set_cppms_path(argv[i] + 8);
      }
      else if(0 == std::strncmp("--namespace=", argv[i], 12))
      {
         app_ptr->set_namespace(argv[i] + 12);
      }
      else if(0 == std::strncmp("--namespace-alias", argv[i], 17))
      {
         app_ptr->set_namespace_alias(true);
      }
      else if(argv[i][0] == '-')
      {
         std::cout << "Error: Unknown argument provided: " << argv[i] << std::endl;
         print_usage_info();
         return 1;
      }
      else
      {
         positional_args.push_back(argv[i]);
      }
   }

   // Handle positional args last:
   for(std::list<const char*>::const_iterator i = positional_args.begin();
      i != positional_args.end(); ++i)
   {
      if(i == --positional_args.end())
         app_ptr->set_destination(*i);
   }

   // Terminate if all required arguments are not provided
   if (app_ptr->get_cppms_path() == "" || app_ptr->get_destination() == "" || app_ptr->get_namespace() == "")
   {
       std::cout << "Error: Please provide all required arguments. " << std::endl;
       return 1;
   }

   return app_ptr->run();
}