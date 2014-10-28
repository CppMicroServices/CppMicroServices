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

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <usSharedLibrary.h>

#include <usTestingConfig.h>

#include "usTestingMacros.h"

#include <cassert>

#include <set>

#ifdef US_PLATFORM_WINDOWS
  static const std::string LIB_PATH = US_RUNTIME_OUTPUT_DIRECTORY;
#else
  static const std::string LIB_PATH = US_LIBRARY_OUTPUT_DIRECTORY;
#endif

US_USE_NAMESPACE

namespace {

  // Please confirm that a character count differing from the following targets is not due to
  // a misconfiguration of your versioning software (Correct line endings for your system)
  // See issue #18 ( https://github.com/saschazelzer/CppMicroServices/issues/18 )
void checkResourceInfo(const ModuleResource& res, const std::string& path,
                       const std::string& baseName,
                       const std::string& completeBaseName, const std::string& suffix,
                       const std::string& completeSuffix,
                       int size, bool children = false)
{
  US_TEST_CONDITION_REQUIRED(res.IsValid(), "Valid resource")
  US_TEST_CONDITION(res.GetBaseName() == baseName, "GetBaseName()")
  US_TEST_CONDITION(res.GetChildren().empty() == !children, "No children")
  US_TEST_CONDITION(res.GetCompleteBaseName() == completeBaseName, "GetCompleteBaseName()")
  US_TEST_CONDITION(res.GetName() == completeBaseName + "." + suffix, "GetName()")
  US_TEST_CONDITION(res.GetResourcePath() == path + completeBaseName + "." + suffix, "GetResourcePath()")
  US_TEST_CONDITION(res.GetPath() == path, "GetPath()")
  US_TEST_CONDITION(res.GetSize() == size, "Data size")
  US_TEST_CONDITION(res.GetSuffix() == suffix, "Suffix")
  US_TEST_CONDITION(res.GetCompleteSuffix() == completeSuffix, "Complete suffix")
}

void testTextResource(Module* module)
{
  ModuleResource res = module->GetResource("foo.txt");
#ifdef US_PLATFORM_WINDOWS
  checkResourceInfo(res, "/", "foo", "foo", "txt", "txt", 16, false);
  const std::streampos ssize(13);
  const std::string fileData = "foo and\nbar\n\n";
#else
  checkResourceInfo(res, "/", "foo", "foo", "txt", "txt", 13, false);
  const std::streampos ssize(12);
  const std::string fileData = "foo and\nbar\n";
#endif

  ModuleResourceStream rs(res);

  rs.seekg(0, std::ios::end);
  US_TEST_CONDITION(rs.tellg() == ssize, "Stream content length");
  rs.seekg(0, std::ios::beg);

  std::string content;
  content.reserve(res.GetSize());
  char buffer[1024];
  while (rs.read(buffer, sizeof(buffer)))
  {
    content.append(buffer, sizeof(buffer));
  }
  content.append(buffer, static_cast<std::size_t>(rs.gcount()));

  US_TEST_CONDITION(rs.eof(), "EOF check");
  US_TEST_CONDITION(content == fileData, "Resource content");

  rs.clear();
  rs.seekg(0);

  US_TEST_CONDITION_REQUIRED(rs.tellg() == std::streampos(0), "Move to start")
  US_TEST_CONDITION_REQUIRED(rs.good(), "Start re-reading");

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(rs, line))
  {
    lines.push_back(line);
  }
  US_TEST_CONDITION_REQUIRED(lines.size() > 1, "Number of lines")
  US_TEST_CONDITION(lines[0] == "foo and", "Check first line")
  US_TEST_CONDITION(lines[1] == "bar", "Check second line")
}

void testTextResourceAsBinary(Module* module)
{
  ModuleResource res = module->GetResource("foo.txt");

#ifdef US_PLATFORM_WINDOWS
  checkResourceInfo(res, "/", "foo", "foo", "txt", "txt", 16, false);
  const std::streampos ssize(16);
  const std::string fileData = "foo and\r\nbar\r\n\r\n";
#else
  checkResourceInfo(res, "/", "foo", "foo", "txt", "txt", 13, false);
  const std::streampos ssize(13);
  const std::string fileData = "foo and\nbar\n\n";
#endif


  ModuleResourceStream rs(res, std::ios_base::binary);

  rs.seekg(0, std::ios::end);
  US_TEST_CONDITION(rs.tellg() == ssize, "Stream content length");
  rs.seekg(0, std::ios::beg);

  std::string content;
  content.reserve(res.GetSize());
  char buffer[1024];
  while (rs.read(buffer, sizeof(buffer)))
  {
    content.append(buffer, sizeof(buffer));
  }
  content.append(buffer, static_cast<std::size_t>(rs.gcount()));

  US_TEST_CONDITION(rs.eof(), "EOF check");
  US_TEST_CONDITION(content == fileData, "Resource content");
}

void testInvalidResource(Module* module)
{
  ModuleResource res = module->GetResource("invalid");
  US_TEST_CONDITION_REQUIRED(res.IsValid() == false, "Check invalid resource")
  US_TEST_CONDITION(res.GetName().empty(), "Check empty name")
  US_TEST_CONDITION(res.GetPath().empty(), "Check empty path")
  US_TEST_CONDITION(res.GetResourcePath().empty(), "Check empty resource path")
  US_TEST_CONDITION(res.GetBaseName().empty(), "Check empty base name")
  US_TEST_CONDITION(res.GetCompleteBaseName().empty(), "Check empty complete base name")
  US_TEST_CONDITION(res.GetSuffix().empty(), "Check empty suffix")

  US_TEST_CONDITION(res.GetChildren().empty(), "Check empty children")
  US_TEST_CONDITION(res.GetSize() == 0, "Check zero size")

  ModuleResourceStream rs(res);
  US_TEST_CONDITION(rs.good() == true, "Check invalid resource stream")
  rs.ignore();
  US_TEST_CONDITION(rs.good() == false, "Check invalid resource stream")
  US_TEST_CONDITION(rs.eof() == true, "Check invalid resource stream")
}

void testSpecialCharacters(Module* module)
{
  ModuleResource res = module->GetResource("special_chars.dummy.txt");
#ifdef US_PLATFORM_WINDOWS
  checkResourceInfo(res, "/", "special_chars", "special_chars.dummy", "txt", "dummy.txt", 56, false);
  const std::streampos ssize(54);
  const std::string fileData = "German Füße (feet)\nFrench garçon de café (waiter)\n";
#else
  checkResourceInfo(res, "/", "special_chars", "special_chars.dummy", "txt", "dummy.txt", 54, false);
  const std::streampos ssize(53);
  const std::string fileData = "German Füße (feet)\nFrench garçon de café (waiter)";
#endif

  ModuleResourceStream rs(res);

  rs.seekg(0, std::ios_base::end);
  US_TEST_CONDITION(rs.tellg() == ssize, "Stream content length");
  rs.seekg(0, std::ios_base::beg);

  std::string content;
  content.reserve(res.GetSize());
  char buffer[1024];
  while (rs.read(buffer, sizeof(buffer)))
  {
    content.append(buffer, sizeof(buffer));
  }
  content.append(buffer, static_cast<std::size_t>(rs.gcount()));

  US_TEST_CONDITION(rs.eof(), "EOF check");
  US_TEST_CONDITION(content == fileData, "Resource content");
}

void testBinaryResource(Module* module)
{
  ModuleResource res = module->GetResource("/icons/cppmicroservices.png");
  checkResourceInfo(res, "/icons/", "cppmicroservices", "cppmicroservices", "png", "png", 2424, false);

  ModuleResourceStream rs(res, std::ios_base::binary);
  rs.seekg(0, std::ios_base::end);
  std::streampos resLength = rs.tellg();
  rs.seekg(0);

  std::ifstream png(US_CORE_SOURCE_DIR "/test/modules/libRWithResources/resources/icons/cppmicroservices.png",
                    std::ifstream::in | std::ifstream::binary);

  US_TEST_CONDITION_REQUIRED(png.is_open(), "Open reference file")

  png.seekg(0, std::ios_base::end);
  std::streampos pngLength = png.tellg();
  png.seekg(0);
  US_TEST_CONDITION(res.GetSize() == resLength, "Check resource size")
  US_TEST_CONDITION_REQUIRED(resLength == pngLength, "Compare sizes")

  char c1 = 0;
  char c2 = 0;
  bool isEqual = true;
  int count = 0;
  while (png.get(c1) && rs.get(c2))
  {
    ++count;
    if (c1 != c2)
    {
      isEqual = false;
      break;
    }
  }

  US_TEST_CONDITION_REQUIRED(count == pngLength, "Check if everything was read");
  US_TEST_CONDITION_REQUIRED(isEqual, "Equal binary contents");
  US_TEST_CONDITION(png.eof(), "EOF check");
}

void testCompressedResource(Module* module)
{
  ModuleResource res = module->GetResource("/icons/compressable.bmp");
  checkResourceInfo(res, "/icons/", "compressable", "compressable", "bmp", "bmp", 300122, false);

  ModuleResourceStream rs(res, std::ios_base::binary);
  rs.seekg(0, std::ios_base::end);
  std::streampos resLength = rs.tellg();
  rs.seekg(0);

  std::ifstream bmp(US_CORE_SOURCE_DIR "/test/modules/libRWithResources/resources/icons/compressable.bmp",
                    std::ifstream::in | std::ifstream::binary);

  US_TEST_CONDITION_REQUIRED(bmp.is_open(), "Open reference file")

  bmp.seekg(0, std::ios_base::end);
  std::streampos bmpLength = bmp.tellg();
  bmp.seekg(0);
  US_TEST_CONDITION(300122 == resLength, "Check resource size")
  US_TEST_CONDITION_REQUIRED(resLength == bmpLength, "Compare sizes")

  char c1 = 0;
  char c2 = 0;
  bool isEqual = true;
  int count = 0;
  while (bmp.get(c1) && rs.get(c2))
  {
    ++count;
    if (c1 != c2)
    {
      isEqual = false;
      break;
    }
  }

  US_TEST_CONDITION_REQUIRED(count == bmpLength, "Check if everything was read");
  US_TEST_CONDITION_REQUIRED(isEqual, "Equal binary contents");
  US_TEST_CONDITION(bmp.eof(), "EOF check");
}

struct ResourceComparator {
  bool operator()(const ModuleResource& mr1, const ModuleResource& mr2) const
  {
    return mr1 < mr2;
  }
};

void testResourceTree(Module* module)
{
  ModuleResource res = module->GetResource("");
  US_TEST_CONDITION(res.GetResourcePath() == "/", "Check root file path")
  US_TEST_CONDITION(res.IsDir() == true, "Check type")

  std::vector<std::string> children = res.GetChildren();
  std::sort(children.begin(), children.end());
  US_TEST_CONDITION_REQUIRED(children.size() == 5, "Check child count")
  US_TEST_CONDITION(children[0] == "foo.txt", "Check child name")
  US_TEST_CONDITION(children[1] == "foo2.txt", "Check child name")
  US_TEST_CONDITION(children[2] == "icons/", "Check child name")
  US_TEST_CONDITION(children[3] == "special_chars.dummy.txt", "Check child name")
  US_TEST_CONDITION(children[4] == "test.xml", "Check child name")

  US_TEST_CONDITION(module->FindResources("!$noexist=?", std::string(), "true").empty(), "Check not existant path");


  ModuleResource readme = module->GetResource("/icons/readme.txt");
  US_TEST_CONDITION(readme.IsFile() && readme.GetChildren().empty(), "Check file resource")

  ModuleResource icons = module->GetResource("icons/");
  US_TEST_CONDITION(icons.IsDir() && !icons.IsFile() && !icons.GetChildren().empty(), "Check directory resource")

  children = icons.GetChildren();
  US_TEST_CONDITION_REQUIRED(children.size() == 3, "Check icons child count")
  std::sort(children.begin(), children.end());
  US_TEST_CONDITION(children[0] == "compressable.bmp", "Check child name")
  US_TEST_CONDITION(children[1] == "cppmicroservices.png", "Check child name")
  US_TEST_CONDITION(children[2] == "readme.txt", "Check child name")

  ResourceComparator resourceComparator;

  // find all .txt files
  std::vector<ModuleResource> nodes = module->FindResources("", "*.txt", false);
  std::sort(nodes.begin(), nodes.end(), resourceComparator);
  US_TEST_CONDITION_REQUIRED(nodes.size() == 3, "Found child count")
  US_TEST_CONDITION(nodes[0].GetResourcePath() == "/foo.txt", "Check child name")
  US_TEST_CONDITION(nodes[1].GetResourcePath() == "/foo2.txt", "Check child name")
  US_TEST_CONDITION(nodes[2].GetResourcePath() == "/special_chars.dummy.txt", "Check child name")

  nodes = module->FindResources("", "*.txt", true);
  std::sort(nodes.begin(), nodes.end(), resourceComparator);
  US_TEST_CONDITION_REQUIRED(nodes.size() == 4, "Found child count")
  US_TEST_CONDITION(nodes[0].GetResourcePath() == "/foo.txt", "Check child name")
  US_TEST_CONDITION(nodes[1].GetResourcePath() == "/foo2.txt", "Check child name")
  US_TEST_CONDITION(nodes[2].GetResourcePath() == "/icons/readme.txt", "Check child name")
  US_TEST_CONDITION(nodes[3].GetResourcePath() == "/special_chars.dummy.txt", "Check child name")

  // find all resources
  nodes = module->FindResources("", "", true);
  US_TEST_CONDITION(nodes.size() == 8, "Total resource number")
  nodes = module->FindResources("", "**", true);
  US_TEST_CONDITION(nodes.size() == 8, "Total resource number")


  // test pattern matching
  nodes.clear();
  nodes = module->FindResources("/icons", "*micro*.png", false);
  US_TEST_CONDITION(nodes.size() == 1 && nodes[0].GetResourcePath() == "/icons/cppmicroservices.png", "Check file pattern matches")

  nodes.clear();
  nodes = module->FindResources("", "*.txt", true);
  US_TEST_CONDITION(nodes.size() == 4, "Check recursive pattern matches")
}

void testResourceOperators(Module* module)
{
  ModuleResource invalid = module->GetResource("invalid");
  ModuleResource foo = module->GetResource("foo.txt");
  US_TEST_CONDITION_REQUIRED(foo.IsValid() && foo, "Check valid resource")
  ModuleResource foo2(foo);
  US_TEST_CONDITION(foo == foo, "Check equality operator")
  US_TEST_CONDITION(foo == foo2, "Check copy constructor and equality operator")
  US_TEST_CONDITION(foo != invalid, "Check inequality with invalid resource")

  ModuleResource xml = module->GetResource("/test.xml");
  US_TEST_CONDITION_REQUIRED(xml.IsValid() && xml, "Check valid resource")
  US_TEST_CONDITION(foo != xml, "Check inequality")
  US_TEST_CONDITION(foo < xml, "Check operator<")

  // check operator< by using a set
  std::set<ModuleResource> resources;
  resources.insert(foo);
  resources.insert(foo);
  resources.insert(xml);
  US_TEST_CONDITION(resources.size() == 2, "Check operator< with set")

  // check hash function specialization
  US_UNORDERED_SET_TYPE<ModuleResource> resources2;
  resources2.insert(foo);
  resources2.insert(foo);
  resources2.insert(xml);
  US_TEST_CONDITION(resources2.size() == 2, "Check operator< with unordered set")

  // check operator<<
  std::ostringstream oss;
  oss << foo;
  US_TEST_CONDITION(oss.str() == foo.GetResourcePath(), "Check operator<<")
}

void testResourceFromExecutable(Module* module)
{
  ModuleResource resource = module->GetResource("usTestResource.txt");
  US_TEST_CONDITION_REQUIRED(resource.IsValid(), "Check valid executable resource")

  std::string line;
  ModuleResourceStream rs(resource);
  std::getline(rs, line);
  US_TEST_CONDITION(line == "meant to be compiled into the test driver", "Check executable resource content")
}

void testResourcesFrom(const std::string& moduleName)
{
#ifdef US_BUILD_SHARED_LIBS
  SharedLibrary libR(LIB_PATH, moduleName);
  try
  {
    libR.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
#endif

  Module* moduleR = ModuleRegistry::GetModule(moduleName);
  US_TEST_CONDITION_REQUIRED(moduleR != NULL, "Test for existing module")

  US_TEST_CONDITION(moduleR->GetName() == moduleName, "Test module name")

  US_TEST_CONDITION(moduleR->FindResources("", "*.txt", true).size() == 2, "Resource count")

#ifdef US_BUILD_SHARED_LIBS
  libR.Unload();
#endif
}

} // end unnamed namespace


int usModuleResourceTest(int /*argc*/, char* /*argv*/[])
{
  US_TEST_BEGIN("ModuleResourceTest");

  ModuleContext* mc = GetModuleContext();
  assert(mc);

#ifdef US_BUILD_SHARED_LIBS
  SharedLibrary libR(LIB_PATH, "TestModuleR");
  try
  {
    libR.Load();
  }
  catch (const std::exception& e)
  {
    US_TEST_FAILED_MSG(<< "Load module exception: " << e.what())
  }
#endif

  Module* moduleR = ModuleRegistry::GetModule("TestModuleR");
  US_TEST_CONDITION_REQUIRED(moduleR != NULL, "Test for existing module TestModuleR")

  US_TEST_CONDITION(moduleR->GetName() == "TestModuleR", "Test module name")

  testInvalidResource(moduleR);

  testResourceFromExecutable(mc->GetModule());

  testResourceTree(moduleR);

  testResourceOperators(moduleR);

  testTextResource(moduleR);
  testTextResourceAsBinary(moduleR);
  testSpecialCharacters(moduleR);

  testBinaryResource(moduleR);

  testCompressedResource(moduleR);

  ModuleResource foo = moduleR->GetResource("foo.txt");
  US_TEST_CONDITION(foo.IsValid() == true, "Valid resource")
#ifdef US_BUILD_SHARED_LIBS
  libR.Unload();
  US_TEST_CONDITION(foo.IsValid() == true, "Still valid resource")
#endif

  testResourcesFrom("TestModuleRL");
  testResourcesFrom("TestModuleRA");

  US_TEST_END()
}
