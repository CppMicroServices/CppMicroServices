/*=============================================================================

  Library: CppMicroServices

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

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

#include "usConfig.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <stack>

#include "stdint_p.h"

#include "usConfig.h"

#ifdef US_PLATFORM_WINDOWS
static const char DIR_SEP = '\\';
#else
static const char DIR_SEP = '/';
#endif

class ResourceWriter;

class Resource
{

public:

  enum Flags
  {
    NoFlags = 0x00,
    Directory = 0x01
  };

  Resource(const std::string& name, const std::string& path = std::string(), unsigned int flags = NoFlags);
  ~Resource();

  std::string GetResourcePath() const;

  int64_t WriteName(ResourceWriter& writer, int64_t offset);
  void WriteTreeInfo(ResourceWriter& writer);
  int64_t WritePayload(ResourceWriter& writer, int64_t offset, std::string* errorMessage);

  std::string name;
  std::string path;
  unsigned int flags;
  Resource* parent;
  std::map<std::string, Resource*> children;
  std::map<uint32_t, Resource*> sortedChildren;

  int64_t nameOffset;
  int64_t dataOffset;
  int64_t childOffset;

};

class ResourceWriter
{

public:

  ResourceWriter(const std::string& fileName, const std::string& libName);
  ~ResourceWriter();

  bool AddFiles(const std::vector<std::string>& files, const std::string& basePath);
  bool Write();

private:

  friend class Resource;

  bool AddFile(const std::string& alias, const Resource& file);

  bool WriteHeader();
  bool WritePayloads();
  bool WriteNames();
  bool WriteDataTree();
  bool WriteRegistrationCode();

  void WriteString(const std::string& str);
  void WriteChar(char c);
  void WriteHex(uint8_t tmp);
  void WriteNumber2(uint16_t number);
  void WriteNumber4(uint32_t number);

  std::ofstream out;
  std::vector<std::string> files;

  std::string libName;
  std::string fileName;

  Resource* root;
};

Resource::Resource(const std::string& name, const std::string& path, unsigned int flags)
  : name(name)
  , path(path)
  , flags(flags)
  , parent(NULL)
  , nameOffset(0)
  , dataOffset(0)
  , childOffset(0)
{
}

Resource::~Resource()
{
  for (std::map<std::string, Resource*>::iterator i = children.begin();
       i != children.end(); ++i)
  {
    delete i->second;
  }
}

std::string Resource::GetResourcePath() const
{
  std::string resource = name;
  for (Resource* p = parent; p; p = p->parent)
  {
    resource = resource.insert(0, p->name + '/');
  }
  return resource;
}

int64_t Resource::WriteName(ResourceWriter& writer, int64_t offset)
{
  // capture the offset
  nameOffset = offset;

  // write the resource name as a comment
  writer.WriteString("  // ");
  writer.WriteString(name);
  writer.WriteString("\n  ");

  // write the length of the name
  writer.WriteNumber2(name.size());
  writer.WriteString("\n  ");
  offset += 2;

  // write the hash value
  writer.WriteNumber4(static_cast<uint32_t>(US_HASH_FUNCTION_NAMESPACE::US_HASH_FUNCTION(std::string, name)));
  writer.WriteString("\n  ");
  offset += 4;

  // write the name itself
  for (std::size_t i = 0; i < name.length(); ++i)
  {
    writer.WriteHex(name[i]);
    if (i != 0 && i % 32 == 0)
      writer.WriteString("\n  ");
  }
  offset += name.length();

  // done
  writer.WriteString("\n  ");
  return offset;
}

void Resource::WriteTreeInfo(ResourceWriter& writer)
{
  // write the resource path as a comment
  writer.WriteString("  // ");
  writer.WriteString(GetResourcePath());
  writer.WriteString("\n  ");

  if (flags & Directory)
  {
    // name offset (in the us_resource_name array)
    writer.WriteNumber4(static_cast<uint32_t>(nameOffset));

    // flags
    writer.WriteNumber2(flags);

    // child count
    writer.WriteNumber4(children.size());

    // first child offset (in the us_resource_tree array)
    writer.WriteNumber4(static_cast<uint32_t>(childOffset));
  }
  else
  {
    // name offset
    writer.WriteNumber4(static_cast<uint32_t>(nameOffset));

    // flags
    writer.WriteNumber2(flags);

    // padding (not used)
    writer.WriteNumber4(0);

    // data offset
    writer.WriteNumber4(static_cast<uint32_t>(dataOffset));
  }
  writer.WriteChar('\n');
}

int64_t Resource::WritePayload(ResourceWriter& writer, int64_t offset, std::string* errorMessage)
{
  // capture the offset
  dataOffset = offset;

  // open the resource file on the file system
  std::ifstream file(path.c_str(), std::ifstream::in | std::ifstream::binary);
  if (!file)
  {
    *errorMessage = "File could not be opened: " + path;
    return 0;
  }

  // write the full path of the resource in the file system as a comment
  writer.WriteString("  // ");
  writer.WriteString(path);
  writer.WriteString("\n  ");

  // write the length
  file.seekg(0, std::ifstream::end);
  std::ifstream::pos_type size = file.tellg();
  file.seekg(0);
  writer.WriteNumber4(size);
  writer.WriteString("\n  ");
  offset += 4;

  // write the actual payload
  int charsLeft = 16;
  char c = 0;
  while (file.get(c))
  {
    --charsLeft;
    writer.WriteHex(static_cast<uint8_t>(c));
    if (charsLeft == 0)
    {
      writer.WriteString("\n  ");
      charsLeft = 16;
    }
  }

  offset += size;

  // done
  writer.WriteString("\n  ");
  return offset;
}

ResourceWriter::ResourceWriter(const std::string& fileName, const std::string& libName)
  : libName(libName)
  , fileName(fileName)
  , root(NULL)
{
  out.exceptions(std::ofstream::goodbit);
  out.open(fileName.c_str());
}

ResourceWriter::~ResourceWriter()
{
  delete root;
}

bool ResourceWriter::AddFiles(const std::vector<std::string>& files, const std::string& basePath)
{
  bool success = true;
  for (std::size_t i = 0; i < files.size(); ++i)
  {
    const std::string& file = files[i];
    if (file.size() <= basePath.size() || file.substr(0, basePath.size()) != basePath)
    {
      std::cerr << "File " << file << " is not an absolute path starting with " << basePath << std::endl;
      success = false;
    }
    else
    {
      const std::string relativePath = file.substr(basePath.size());
      std::string name = relativePath;
      std::size_t index = relativePath.find_last_of(DIR_SEP);
      if (index != std::string::npos)
      {
        name = relativePath.substr(index+1);
      }
      success &= AddFile(relativePath, Resource(name, file));
    }
  }
  return success;
}

bool ResourceWriter::Write()
{
  if (!WriteHeader())
  {
    std::cerr << "Could not write header." << std::endl;
    return false;
  }
  if (!WritePayloads())
  {
    std::cerr << "Could not write data blobs." << std::endl;
    return false;
  }
  if (!WriteNames())
  {
    std::cerr << "Could not write file names." << std::endl;
      return false;
  }
  if (!WriteDataTree())
  {
    std::cerr << "Could not write data tree." << std::endl;
    return false;
  }
  if (!WriteRegistrationCode())
  {
    std::cerr << "Could not write footer" << std::endl;
    return false;
  }

  return true;
}

bool ResourceWriter::AddFile(const std::string& alias, const Resource& file)
{
  std::ifstream in(file.path.c_str(), std::ifstream::in | std::ifstream::binary);
  if (!in)
  {
    std::cerr << "File could not be opened: " << file.path << std::endl;
    return false;
  }
  in.seekg(0, std::ifstream::end);
  std::ifstream::pos_type size = in.tellg();
  in.close();

  if (size > 0xffffffff)
  {
    std::cerr << "File too big: " << file.path << std::endl;
    return false;
  }

  if (!root)
  {
    root = new Resource(std::string(), std::string(), Resource::Directory);
  }

  Resource* parent = root;
  std::stringstream ss(alias);
  std::vector<std::string> nodes;
  {
    std::string node;
    while (std::getline(ss, node, DIR_SEP))
    {
      if (node.empty())
        continue;
      nodes.push_back(node);
    }
  }

  for(std::size_t i = 0; i < nodes.size()-1; ++i)
  {
    const std::string& node = nodes[i];
    if (parent->children.find(node) == parent->children.end())
    {
      Resource* s = new Resource(node, std::string(), Resource::Directory);
      s->parent = parent;
      parent->children.insert(std::make_pair(node, s));
      parent->sortedChildren.insert(std::make_pair(static_cast<uint32_t>(US_HASH_FUNCTION_NAMESPACE::US_HASH_FUNCTION(std::string, node)), s));
      parent = s;
    }
    else
    {
      parent = parent->children[node];
    }
  }

  const std::string filename = nodes.back();
  Resource* s = new Resource(file);
  s->parent = parent;
  parent->children.insert(std::make_pair(filename, s));
  parent->sortedChildren.insert(std::make_pair(static_cast<uint32_t>(US_HASH_FUNCTION_NAMESPACE::US_HASH_FUNCTION(std::string, filename)), s));
  return true;
}

bool ResourceWriter::WriteHeader()
{
  std::stringstream ss;
  std::time_t now = time(0);
  ss << std::ctime(&now);

  WriteString("/*=============================================================================\n");
  WriteString("  Resource object code\n");
  WriteString("\n");
  WriteString("  Created: ");
  WriteString(ss.str());
  WriteString("       by: The Resource Compiler for CppMicroServices version ");
  WriteString(CppMicroServices_VERSION_STR);
  WriteString("\n\n");
  WriteString("  WARNING! All changes made in this file will be lost!\n");
  WriteString( "=============================================================================*/\n\n");
  WriteString("#include <usConfig.h>\n");
  WriteString("#include <usModuleInfo.h>\n\n");
  return true;
}

bool ResourceWriter::WritePayloads()
{
  if (!root)
    return false;

  WriteString("static const unsigned char us_resource_data[] = {\n");
  std::stack<Resource*> pending;

  pending.push(root);
  int64_t offset = 0;
  std::string errorMessage;
  while (!pending.empty())
  {
    Resource* file = pending.top();
    pending.pop();
    for (std::map<std::string, Resource*>::iterator i = file->children.begin();
         i != file->children.end(); ++i)
    {
      Resource* child = i->second;
      if (child->flags & Resource::Directory)
      {
        pending.push(child);
      }
      else
      {
        offset = child->WritePayload(*this, offset, &errorMessage);
        if (offset == 0)
        {
          std::cerr << errorMessage << std::endl;
        }
      }
    }
  }
  WriteString("\n};\n\n");
  return true;
}

bool ResourceWriter::WriteNames()
{
  if (!root)
    return false;

  WriteString("static const unsigned char us_resource_name[] = {\n");

  std::map<std::string, int64_t> names;
  std::stack<Resource*> pending;

  pending.push(root);
  int64_t offset = 0;
  while (!pending.empty())
  {
    Resource* file = pending.top();
    pending.pop();
    for (std::map<std::string, Resource*>::iterator it = file->children.begin();
         it != file->children.end(); ++it)
    {
      Resource* child = it->second;
      if (child->flags & Resource::Directory)
      {
        pending.push(child);
      }
      if (names.find(child->name) != names.end())
      {
        child->nameOffset = names[child->name];
      }
      else
      {
        names.insert(std::make_pair(child->name, offset));
        offset = child->WriteName(*this, offset);
      }
    }
  }
  WriteString("\n};\n\n");
  return true;
}

bool ResourceWriter::WriteDataTree()
{
  if (!root)
    return false;

  WriteString("static const unsigned char us_resource_tree[] = {\n");
  std::stack<Resource*> pending;

  // calculate the child offsets in the us_resource_tree array
  pending.push(root);
  int offset = 1;
  while (!pending.empty())
  {
    Resource* file = pending.top();
    pending.pop();
    file->childOffset = offset;

    // calculate the offset now
    for (std::map<uint32_t, Resource*>::iterator i = file->sortedChildren.begin();
         i != file->sortedChildren.end(); ++i)
    {
      Resource* child = i->second;
      ++offset;
      if (child->flags & Resource::Directory)
      {
        pending.push(child);
      }
    }
  }

  // write the tree structure
  pending.push(root);
  root->WriteTreeInfo(*this);
  while (!pending.empty())
  {
    Resource *file = pending.top();
    pending.pop();

    // write the actual data now
    for (std::map<uint32_t, Resource*>::iterator i = file->sortedChildren.begin();
         i != file->sortedChildren.end(); ++i)
    {
      Resource *child = i->second;
      child->WriteTreeInfo(*this);
      if (child->flags & Resource::Directory)
      {
        pending.push(child);
      }
    }
  }
  WriteString("\n};\n\n");

  return true;
}

bool ResourceWriter::WriteRegistrationCode()
{
  WriteString("US_BEGIN_NAMESPACE\n\n");
  WriteString("extern US_EXPORT bool RegisterResourceData(int, ModuleInfo*, ModuleInfo::ModuleResourceData, ModuleInfo::ModuleResourceData, ModuleInfo::ModuleResourceData);\n\n");
  WriteString("US_END_NAMESPACE\n\n");

  WriteString(std::string("extern \"C\" US_ABI_EXPORT int _us_init_resources_") + libName +
              "(US_PREPEND_NAMESPACE(ModuleInfo)* moduleInfo)\n");
  WriteString("{\n");
  WriteString("  US_PREPEND_NAMESPACE(RegisterResourceData)(0x01, moduleInfo, us_resource_tree, us_resource_name, us_resource_data);\n");
  WriteString("  return 1;\n");
  WriteString("}\n");

  return true;
}

void ResourceWriter::WriteString(const std::string& str)
{
  out << str;
}

void ResourceWriter::WriteChar(char c)
{
  out << c;
}

void ResourceWriter::WriteHex(uint8_t tmp)
{
  const char* const digits = "0123456789abcdef";
  WriteChar('0');
  WriteChar('x');
  if (tmp < 16)
  {
    WriteChar(digits[tmp]);
  }
  else
  {
    WriteChar(digits[tmp >> 4]);
    WriteChar(digits[tmp & 0xf]);
  }
  WriteChar(',');
}

void ResourceWriter::WriteNumber2(uint16_t number)
{
  WriteHex(number >> 8);
  WriteHex(static_cast<uint8_t>(number));
}

void ResourceWriter::WriteNumber4(uint32_t number)
{
  WriteHex(number >> 24);
  WriteHex(number >> 16);
  WriteHex(number >> 8);
  WriteHex(number);
}


#ifdef US_PLATFORM_POSIX
#include <unistd.h>
std::string GetCurrentDir()
{
  char currDir[512];
  if (!getcwd(currDir, sizeof(currDir)))
  {
    std::cerr << "Getting the current directory failed." << std::endl;
    exit(EXIT_FAILURE);
  }
  return std::string(currDir);
}
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
std::string GetCurrentDir()
{
  TCHAR currDir[512];
  DWORD dwRet;
  dwRet = GetCurrentDirectory(sizeof(currDir), currDir);

  if( dwRet == 0 || dwRet > 512)
  {
    std::cerr << "Getting the current directory failed." << std::endl;
    exit(EXIT_FAILURE);
  }
  return std::string(currDir);
}
#endif

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    std::cout << US_RCC_EXECUTABLE_NAME " - A resource compiler for C++ Micro Services modules\n"
                 "\n"
                 "Usage: " US_RCC_EXECUTABLE_NAME " LIBNAME OUTPUT INPUT [INPUT]...\n"
                 "Convert all INPUT files into hex code embedded in C funtions written to OUTPUT.\n";
    exit(EXIT_SUCCESS);
  }

  std::string libName(argv[1]);
  std::string fileName(argv[2]);

  std::vector<std::string> inputFiles;

  for (int i = 3; i < argc; i++)
  {
    inputFiles.push_back(argv[i]);
  }

  ResourceWriter writer(fileName, libName);
  if (!writer.AddFiles(inputFiles, GetCurrentDir()))
  {
    return EXIT_FAILURE;
  }

  return writer.Write() ? EXIT_SUCCESS : EXIT_FAILURE;
}
