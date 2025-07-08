#include "ChangeNamespace.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>

std::string
ChangeNamespace::read_file_content(fs::path const& file_path)
{
    std::ifstream is(file_path);
    return std::string(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
}

void
ChangeNamespace::write_file_content(fs::path const& file_path, std::string const& content)
{
    std::ofstream os(file_path, std::ios_base::out);
    os << content;
}

std::string
ChangeNamespace::replace_source_patterns(std::string const& content)
{
    const std::vector<std::string> patterns = { R"((namespace\s+)cppmicroservices(_\w+)?)",
                                                R"((<)cppmicroservices((?:_\w+)?\s*(?:::)))",
                                                R"((namespace\s+\w+\s*=\s*(?:::\s*)?)cppmicroservices(_\w+)?)",
                                                R"((^\s*#\s*define\s+\w+\s+)cppmicroservices((?:_\w+)?\s*)$)",
                                                R"((\(\s*)cppmicroservices(\s*\)))",
                                                R"(()cppmicroservices((?:_\w+)?(?:::)))" };

    const std::string replacement = "$1" + m_namespace_name + "$2";
    std::string result = content;

    for (auto const& pattern : patterns)
    {
        std::regex re(pattern);
        result = std::regex_replace(result, re, replacement);
    }

    return result;
}

std::string
ChangeNamespace::add_namespace_alias(std::string const& content)
{
    const std::regex namespace_alias("(namespace)(\\s+)(" + m_namespace_name + ")(\\s*\\{)");
    return std::regex_replace(content, namespace_alias, "$1 $3 {} $1 cppmicroservices = $3; $1$2$3$4");
}

std::string
ChangeNamespace::replace_manifest_pattern(std::string const& content)
{
    const std::string pattern = "(\")cppmicroservices(::([^\"]*)\")";
    const std::string replacement = "$1" + m_namespace_name + "$2";
    std::regex re(pattern);
    return std::regex_replace(content, re, replacement);
}

void
ChangeNamespace::copy_path(fs::path const& p)
{
    assert(!fs::is_directory(m_cppms_src_path / p));
    if (fs::exists(m_dest_path / p))
    {
        std::cout << "Copying (and overwriting) file: " << p.string() << "\n";
        fs::remove(m_dest_path / p);
    }
    else
    {
        std::cout << "Copying file: " << p.string() << "\n";
    }

    // Create the path to the new file
    create_path(p.parent_path());

    if (is_source_file(p) || is_test_config_file(p))
    {
        std::string content = read_file_content(m_cppms_src_path / p);
        content = replace_source_patterns(content);

        if (m_namespace_alias)
        {
            content = add_namespace_alias(content);
        }

        write_file_content(m_dest_path / p, content);
    }
    else if (is_manifest_json_file(p))
    {
        std::string content = read_file_content(m_cppms_src_path / p);
        content = replace_manifest_pattern(content);
        write_file_content(m_dest_path / p, content);
    }
    else
    {
        fs::copy_file(m_cppms_src_path / p, m_dest_path / p);
    }
}

void
ChangeNamespace::create_path(fs::path const& p)
{
    if (!fs::exists(m_dest_path / p))
    {
        // recurse then create the path:
        create_path(p.parent_path());
        fs::create_directory(m_dest_path / p);
    }
}