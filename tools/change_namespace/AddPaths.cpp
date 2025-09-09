#include "ChangeNamespace.hpp"

#include <array>
#include <iostream>
#include <regex>

void
ChangeNamespace::add_path(fs::path const& p)
{
    fs::path normalized_path = p;
    normalized_path = normalized_path.lexically_normal();
    if (fs::exists(m_cppms_src_path / normalized_path))
    {
        if (fs::is_directory(m_cppms_src_path / normalized_path))
        {
            add_directory(normalized_path);
        }
        else
        {
            add_file(normalized_path);
        }
    }
    else
    {
        std::cerr << "Error: dependent file " << p.string() << " does not exist." << std::endl;
    }
}

void
ChangeNamespace::add_directory(fs::path const& p)
{
    if (fs::path(m_cppms_src_path / p) == fs::path(m_cppms_src_path / "tools" / "change_namespace"))
    {
        return;
    }

    // Parse files and directories
    for (auto const& entry : fs::directory_iterator(m_cppms_src_path / p))
    {
        std::string s = entry.path().string();
        s.erase(0, m_cppms_src_path.string().size() + 1);

        fs::path np = s;
        if (!m_dependencies.count(np))
        {
            m_dependencies[np] = p; // set up dependency tree
            add_pending_path(np);
        }
    }
}

void
ChangeNamespace::add_file(fs::path const& p)
{
    // If the file is already parsed, return
    if (m_copy_paths.find(p) != m_copy_paths.end())
    {
        return;
    }

    // Else add the file to our list
    m_copy_paths.insert(p);
}