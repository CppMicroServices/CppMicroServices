#include "ChangeNamespace.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>

ChangeNamespace::ChangeNamespace() : m_namespace_alias(false) {}

bool
ChangeNamespace::is_source_file(fs::path const& p)
{
    static const std::regex e(".*\\."
                              "(?:"
                              "c|cxx|h|hxx|inc|inl|.?pp|yy?"
                              ")",
                              std::regex::icase);
    return std::regex_match(p.filename().generic_string(), e);
}

bool
ChangeNamespace::is_test_config_file(fs::path const& p)
{
    static const std::regex e(".*\\."
                              "(?:"
                              "h.in?"
                              ")",
                              std::regex::icase);
    return std::regex_match(p.filename().generic_string(), e);
}

bool
ChangeNamespace::is_manifest_json_file(fs::path const& p)
{
    static const std::regex e(".*\\."
                              "(?:"
                              "json?"
                              ")",
                              std::regex::icase);
    return std::regex_match(p.filename().generic_string(), e);
}

void
ChangeNamespace::set_cppms_src_path(std::string_view p)
{
    m_cppms_src_path = (fs::path(p) / "CppMicroServices").parent_path();
}

void
ChangeNamespace::set_destination(std::string_view p)
{
    m_dest_path = fs::path(p);
}

void
ChangeNamespace::set_namespace(std::string_view name)
{
    m_namespace_name = name;
}

void
ChangeNamespace::set_namespace_alias(bool b)
{
    m_namespace_alias = b;
}

std::string
ChangeNamespace::get_cppms_path() const
{
    return m_cppms_src_path.string();
}

std::string
ChangeNamespace::get_destination() const
{
    return m_dest_path.string();
}

std::string
ChangeNamespace::get_namespace() const
{
    return m_namespace_name;
}

fs::path
get_short_path(fs::path const& p)
{
    // Truncate path no more than "x/y"
    std::string s = p.generic_string();
    std::string::size_type n = s.find('/');
    if (n != std::string::npos)
    {
        n = s.find('/', n + 1);
        if (n != std::string::npos)
        {
            s.erase(n);
        }
    }
    return s;
}

int
ChangeNamespace::run()
{
    // Check if cppms source path exists
    if (!fs::exists(m_cppms_src_path))
    {
        std::cout << "Error: Cppmicroservices source path does not exist: " << m_cppms_src_path.string() << "\n";
        return 1;
    }

    // Check if output path exists
    if (!fs::exists(m_dest_path))
    {
        std::cout << "Error: Destination path does not exist: " << m_dest_path.string() << "\n";
        return 1;
    }

    // Iterate through all entries in the CppMicroServices directory
    fs::directory_iterator end;
    for (fs::directory_iterator cppms_iter((fs::path(m_cppms_src_path))); cppms_iter != end; ++cppms_iter)
    {
        add_path(cppms_iter->path().filename());
    }

    // Process all pending paths
    while (!m_pending_paths.empty())
    {
        add_path(m_pending_paths.front());
        m_pending_paths.pop();
    }

    // Copy all files that need to be processed including replacing the namespaces
    std::set<fs::path, path_less>::iterator m, n;
    m = m_copy_paths.begin();
    n = m_copy_paths.end();

    while (m != n)
    {
        copy_path(*m);
        ++m;
    }

    return 0;
}
