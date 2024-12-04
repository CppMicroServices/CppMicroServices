#include "ChangeNamespace.hpp"

#include <cstring>
#include <filesystem>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>

namespace fs = std::filesystem;

class FileHandler;

// Path operations
int compare_paths(const fs::path& a, const fs::path& b);
inline bool equal_paths(const fs::path& a, const fs::path& b)
{ return compare_paths(a, b) == 0; }

struct path_less
{
   bool operator()(const fs::path& a, const fs::path& b)const
   { return compare_paths(a, b) < 0; }
};

class ChangeNamespaceImpl : public CNApplication
{
public:
   ChangeNamespaceImpl();
   ~ChangeNamespaceImpl();
   static bool is_source_file(const fs::path& p);
   static bool is_test_config_file(const fs::path& p);
   static bool is_manifest_json_file(const fs::path& p);
private:

   void set_cppms_path(const char* p);
   void set_destination(const char* p);
   void set_namespace(const char* name);
   void set_namespace_alias(bool);
   std::string get_cppms_path();
   std::string get_destination();
   std::string get_namespace();

   virtual int run();

   // Helper functions:
   void add_path(const fs::path& p);
   void add_pending_path(const fs::path& p) { m_pending_paths.push(p); }
   void add_directory(const fs::path& p);
   void add_file(const fs::path& p);
   void copy_path(const fs::path& p);
   void add_file_dependencies(const fs::path& p, bool scanfile);
   void create_path(const fs::path& p);

   bool m_namespace_alias;               // make "cppms" a namespace alias when doing a namespace rename.
   fs::path m_cppms_path;                // the path to the cppms root
   fs::path m_dest_path;                 // the path to copy to
   std::set<fs::path, path_less>                         m_copy_paths;                 // list of files to copy
   std::map<fs::path, fs::path, path_less>               m_dependencies;               // dependency information
   std::string                                           m_namespace_name;             // namespace rename.
   std::queue<fs::path, std::list<fs::path> >            m_pending_paths;              // Queue of paths we haven't scanned yet.
};