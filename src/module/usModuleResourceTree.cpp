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


#include "usModuleResourceTree_p.h"

#include "usUtils_p.h"
#include "stdint_p.h"

#include <iostream>
#include <sstream>

//#define DEBUG_RESOURCE_MATCH

US_BEGIN_NAMESPACE

US_EXPORT bool RegisterResourceData(int, ModuleInfo* moduleInfo,
                                    ModuleInfo::ModuleResourceData resourceTree,
                                    ModuleInfo::ModuleResourceData resourceNames,
                                    ModuleInfo::ModuleResourceData resourceData)
{
  moduleInfo->resourceTree.push_back(resourceTree);
  moduleInfo->resourceNames.push_back(resourceNames);
  moduleInfo->resourceData.push_back(resourceData);
  return true;
}


ModuleResourceTree::ModuleResourceTree(ModuleInfo::ModuleResourceData resourceTree,
                                       ModuleInfo::ModuleResourceData resourceNames,
                                       ModuleInfo::ModuleResourceData resourceData)
  : isValid(resourceTree && resourceNames && resourceData)
  , tree(resourceTree)
  , names(resourceNames)
  , payloads(resourceData)
{
}

bool ModuleResourceTree::IsValid() const
{
  return isValid;
}

void ModuleResourceTree::Invalidate()
{
  isValid = false;
}

inline std::string ModuleResourceTree::GetName(int node) const
{
  if(!node) // root
    return std::string();

  const int offset = FindOffset(node);

  std::string ret;
  int nameOffset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                   (tree[offset+2] << 8) + (tree[offset+3] << 0);
  const int16_t nameLength = (names[nameOffset+0] << 8) +
                             (names[nameOffset+1] << 0);
  nameOffset += 2; // jump past name length
  nameOffset += 4; // jump past hash

  ret.resize(nameLength);
  for(int i = 0; i < nameLength; ++i)
  {
    ret[i] = names[nameOffset+i];
  }
  return ret;
}

int ModuleResourceTree::FindNode(const std::string& _path) const
{
  std::string path = _path.empty() ? "/" : _path;

#ifdef DEBUG_RESOURCE_MATCH
  US_DEBUG << "***" << " START " << path;
#endif

  if(path == "/")
    return 0;

  // the root node is always first
  int childCount = (tree[6] << 24) + (tree[7] << 16) +
                   (tree[8] << 8) + (tree[9] << 0);
  int child = (tree[10] << 24) + (tree[11] << 16) +
              (tree[12] << 8) + (tree[13] << 0);

  int node = -1;

  // split the full path into segments
  std::vector<std::string> segments;
  {
    std::stringstream ss(path);
    std::string item;
    while(std::getline(ss, item, '/'))
    {
      if (item.empty()) continue;
      segments.push_back(item);
    }
  }

  //now iterate up the path hierarchy
  for (std::size_t i = 0; i < segments.size() && childCount; ++i)
  {
    const std::string& segment = segments[i];

#ifdef DEBUG_RESOURCE_MATCH
    US_DEBUG << "  CHILDREN " << segment;
    for(int j = 0; j < childCount; ++j)
    {
      US_DEBUG << "   " << child+j << " :: " << GetName(child+j);
    }
#endif

    // get the hash value for the current segment
    const uint32_t currHash = static_cast<uint32_t>(US_HASH_FUNCTION_NAMESPACE::US_HASH_FUNCTION(std::string,segment));

    // do a binary search for the hash
    int l = 0;
    int r = childCount-1;
    int subNode = (l+r+1)/2;
    while(r != l)
    {
      const uint32_t subNodeHash = GetHash(child+subNode);
      if(currHash == subNodeHash)
        break;
      else if(currHash < subNodeHash)
        r = subNode - 1;
      else
        l = subNode;
      subNode = (l+r+1) / 2;
    }
    subNode += child;

    // now check fo collisions and do compare using equality
    bool found = false;
    if(GetHash(subNode) == currHash)
    {
      while(subNode > child && GetHash(subNode-1) == currHash) // walk up to include all collisions
        --subNode;
      for(; subNode < child+childCount && GetHash(subNode) == currHash; ++subNode)
      {
        // now test using name comparison
        if(GetName(subNode) == segment)
        {
          found = true;
          int offset = FindOffset(subNode);
#ifdef DEBUG_RESOURCE_MATCH
          US_DEBUG << "  TRY " << subNode << " " << GetName(subNode) << " " << offset;
#endif
          offset += 4;  // jump past name

          const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
          offset += 2;

          if(i == segments.size()-1)
          {
#ifdef DEBUG_RESOURCE_MATCH
            US_DEBUG << "!!!!" << " FINISHED " << subNode;
#endif
            return subNode;
          }

          // if we are not at the end of the resource path and the current
          // segment is not a directory, return "not found" (this shouldn't happen).
          if(!(flags & Directory))
            return -1;

          childCount = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                       (tree[offset+2] << 8) + (tree[offset+3] << 0);
          offset += 4;
          child = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                  (tree[offset+2] << 8) + (tree[offset+3] << 0);
          break;
        }
      }
    }
    if(!found)
    {
      break;
    }
  }
#ifdef DEBUG_RESOURCE_MATCH
  US_DEBUG << "***" << " FINISHED " << node;
#endif
  return node;
}

void ModuleResourceTree::FindNodes(const std::string& _path,
                                   const std::string& _filePattern,
                                   bool recurse, std::vector<std::string>& result)
{
  std::string path = _path.empty() ? std::string("/") : _path;
  if (path[path.size()-1] != '/') path.append("/");

  std::string filePattern = _filePattern.empty() ? std::string("*") : _filePattern;

  int rootNode = FindNode(path);
  if (rootNode > -1)
  {
    this->FindNodes(result, path, rootNode, filePattern, recurse);
  }
}

short ModuleResourceTree::GetFlags(int node) const
{
  if(node == -1)
    return 0;
  const int offset = FindOffset(node) + 4; //jump past name
  return (tree[offset+0] << 8) + (tree[offset+1] << 0);
}

void ModuleResourceTree::FindNodes(std::vector<std::string> &result, const std::string& path,
                                   const int rootNode,
                                   const std::string &filePattern, bool recurse)
{
  int offset = FindOffset(rootNode) + 6; // jump past name and type

  const int childCount = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                         (tree[offset+2] << 8) + (tree[offset+3] << 0);
  offset += 4;
  const int childNode = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                        (tree[offset+2] << 8) + (tree[offset+3] << 0);

  for(int i = childNode; i < childNode+childCount; ++i)
  {
    const int childOffset = FindOffset(i);
    const short childFlags = (tree[childOffset+4] << 8) + (tree[childOffset+5] << 0);
    if (!(childFlags & Directory))
    {
      const std::string name = GetName(i);
      if (this->Matches(name, filePattern))
      {
        result.push_back(path + name);
      }
    }
    else if (recurse)
    {
      this->FindNodes(result, path + GetName(i) + "/", i, filePattern, recurse);
    }
  }
}

uint32_t ModuleResourceTree::GetHash(int node) const
{
  if(!node) // root node
    return 0;

  const int offset = FindOffset(node);
  int nameOffset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                   (tree[offset+2] << 8) + (tree[offset+3] << 0);
  nameOffset += 2; // jump past name length
  return (names[nameOffset+0] << 24) + (names[nameOffset+1] << 16) +
         (names[nameOffset+2] << 8) + (names[nameOffset+3] << 0);
}

bool ModuleResourceTree::Matches(const std::string &name, const std::string &filePattern)
{
  // short-cut
  if (filePattern == "*") return true;

  std::stringstream ss(filePattern);
  std::string tok;
  std::size_t pos = 0;
  while(std::getline(ss, tok, '*'))
  {
    std::size_t index = name.find(tok, pos);
    if (index == std::string::npos) return false;
    pos = index + tok.size();
  }
  return true;
}

const unsigned char* ModuleResourceTree::GetData(int node, int32_t* size) const
{
  if(node == -1)
  {
    *size = 0;
    return 0;
  }
  int offset = FindOffset(node) + 4; // jump past name

  const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
  offset += 2;

  offset += 4; // jump past the padding

  if(!(flags & Directory))
  {
    const int dataOffset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                           (tree[offset+2] << 8) + (tree[offset+3] << 0);
    const int32_t dataLength = (payloads[dataOffset+0] << 24) + (payloads[dataOffset+1] << 16) +
                               (payloads[dataOffset+2] << 8) + (payloads[dataOffset+3] << 0);
    const unsigned char* ret = payloads+dataOffset+4;
    *size = dataLength;
    return ret;
  }
  *size = 0;
  return 0;
}

void ModuleResourceTree::GetChildren(int node, std::vector<std::string>& ret) const
{
  if(node == -1)
    return;

  int offset = FindOffset(node) + 4; // jump past name

  const short flags = (tree[offset+0] << 8) + (tree[offset+1] << 0);
  offset += 2;

  if(flags & Directory)
  {
    const int childCount = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                           (tree[offset+2] << 8) + (tree[offset+3] << 0);
    offset += 4;
    const int childOffset = (tree[offset+0] << 24) + (tree[offset+1] << 16) +
                            (tree[offset+2] << 8) + (tree[offset+3] << 0);
    for(int i = childOffset; i < childOffset+childCount; ++i)
    {
      ret.push_back(GetName(i));
    }
  }
}

US_END_NAMESPACE
