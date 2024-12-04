#include "ChangeNamespaceImpl.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

ChangeNamespaceImpl::ChangeNamespaceImpl()
  : m_namespace_alias(false)
{
}

ChangeNamespaceImpl::~ChangeNamespaceImpl()
{
}

CNApplication::~CNApplication()
{
}

bool ChangeNamespaceImpl::is_source_file(const fs::path& p)
{
    static const std::regex e(
        ".*\\."
        "(?:"
            "c|cxx|h|hxx|inc|inl|.?pp|yy?"
        ")", 
        std::regex::icase
    );
    return std::regex_match(p.filename().generic_string(), e);
}

bool ChangeNamespaceImpl::is_test_config_file(const fs::path& p)
{
    static const std::regex e(
        ".*\\."
        "(?:"
            "h.in?"
        ")", 
        std::regex::icase
    );
    return std::regex_match(p.filename().generic_string(), e);
}

bool ChangeNamespaceImpl::is_manifest_json_file(const fs::path& p)
{
    static const std::regex e(
        ".*\\."
        "(?:"
            "json?"
        ")", 
        std::regex::icase
    );
    return std::regex_match(p.filename().generic_string(), e);
}

void ChangeNamespaceImpl::set_cppms_path(const char* p)
{
   m_cppms_path = (fs::path(p) / "CppMicroServices").parent_path(); 
}

void ChangeNamespaceImpl::set_destination(const char* p)
{
   m_dest_path = fs::path(p);
}

void ChangeNamespaceImpl::set_namespace(const char* name)
{
   m_namespace_name = name;
}

void ChangeNamespaceImpl::set_namespace_alias(bool b)
{
   m_namespace_alias = b;
}

std::string ChangeNamespaceImpl::get_cppms_path()
{
    return m_cppms_path.string();
}

std::string ChangeNamespaceImpl::get_destination()
{
    return m_dest_path.string();
}

std::string ChangeNamespaceImpl::get_namespace()
{
    return m_namespace_name;
}

fs::path get_short_path(const fs::path& p)
{
   // Truncate path no more than "x/y"
   std::string s = p.generic_string();
   std::string::size_type n = s.find('/');
   if(n != std::string::npos)
   {
      n = s.find('/', n+1);
      if(n != std::string::npos)
         s.erase(n);
   }
   return s;
}

int ChangeNamespaceImpl::run()
{
   // Check if output path exists
   if(!fs::exists(m_dest_path))
   {
      std::string msg("Destination path does not exist: ");
      msg.append(m_dest_path.string());
      throw std::runtime_error(msg);
   }

   fs::directory_iterator end;
   for (fs::directory_iterator cppms_iter((fs::path(m_cppms_path))); cppms_iter != end; ++cppms_iter){
       add_path(cppms_iter->path().filename());
   }
   
   while (!m_pending_paths.empty())
   {
       add_path(m_pending_paths.front());
       m_pending_paths.pop();
   }

   std::set<fs::path, path_less>::iterator m, n;
   m = m_copy_paths.begin();
   n = m_copy_paths.end();

   while(m != n)
   {
       copy_path(*m);
       ++m;
   }

   return 0;
}

cn_app_ptr CNApplication::create()
{
   cn_app_ptr result(static_cast<CNApplication*>(new ChangeNamespaceImpl()));
   return result;
}
