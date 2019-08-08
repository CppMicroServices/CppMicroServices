/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

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
#ifndef COMPONENTCALLBACKGENERATOR_HPP
#define COMPONENTCALLBACKGENERATOR_HPP

#include <fstream>
#include <sstream>

#include "gtest/gtest_prod.h"
#include "ComponentInfo.hpp"
#include "Util.hpp"

using codegen::datamodel::ComponentInfo;

namespace codegen
{

class ComponentCallbackGenerator
{
public:
  ComponentCallbackGenerator(const std::vector<std::string>& includeHeaders,
                             const std::vector<ComponentInfo>& componentInfos)
    : mHeaderIncludes(includeHeaders)
    , mComponentInfos(componentInfos)
    , mStrStream()
  {
    Substitute();
  }

  std::string GetString() const
  {
    return mStrStream.str();
  }

private:
  FRIEND_TEST(CodeGenTest, TestSubstitution);
  FRIEND_TEST(CodeGenTest, TestSubstitutionDynamic);
  FRIEND_TEST(CodeGenTest, TestMultipleComponents);

  void Substitute()
  {
    SubstituteHeader();
    SubstituteBody();
  }

  void SubstituteHeader()
  {
    const std::string includes = R"cpptemplate(
#include <vector>
#include <cppmicroservices/ServiceInterface.h>
#include "ServiceComponent/detail/ComponentInstanceImpl.hpp"
)cpptemplate";
    mStrStream << includes;

    for (const auto& header : mHeaderIncludes)
    {
      mStrStream << codegen::util::Substitute("#include \"{0}\"", header) << "\n";
    }

    const std::string namespaces = R"cpptemplate(
namespace sc = cppmicroservices::service::component;
namespace scd = cppmicroservices::service::component::detail;
using scd::ComponentInstance;
using scd::ComponentInstanceImpl;
using scd::Binder;
using scd::StaticBinder;
using scd::DynamicBinder;
)cpptemplate";
    mStrStream << namespaces;
  }

  void SubstituteBody()
  {
    for (const auto& componentInfo : mComponentInfos)
    {
      const std::string createFuncPartBegin = R"cpptemplate(
extern "C" US_ABI_EXPORT ComponentInstance* NewInstance_{0}()
{)cpptemplate";
      mStrStream << codegen::util::Substitute(createFuncPartBegin, datamodel::GetComponentNameStr(componentInfo)) << "\n";

      auto isReferencesEmpty = componentInfo.references.empty();
      if(!isReferencesEmpty)
      {
        std::string binders = "  std::vector<std::shared_ptr<Binder<{0}>>> binders;";
        mStrStream << codegen::util::Substitute(binders, componentInfo.implClassName) << "\n";
      }

      for(const auto& ref : componentInfo.references)
      {
        std::string refbinderStr = "  binders.push_back(" + datamodel::GetReferenceBinderStr(ref) + ");";
        mStrStream << codegen::util::Substitute(refbinderStr, componentInfo.implClassName) << "\n";
      }

      std::string compInstance = "  ComponentInstance* componentInstance = new (std::nothrow) ComponentInstanceImpl<" + componentInfo.implClassName;
      compInstance.append(", std::tuple<" + datamodel::GetServiceInterfacesStr(componentInfo.service) + ">");
      compInstance.append(", " + datamodel::GetInjectReferencesStr(componentInfo));
      compInstance.append(isReferencesEmpty ? ">();": ", " + datamodel::GetReferencesStr(componentInfo) + ">(binders);");
      mStrStream << compInstance << "\n";

      const std::string createFuncPartEnd = R"cpptemplate(
  return componentInstance;
})cpptemplate";
      mStrStream << createFuncPartEnd << "\n";

      const std::string deleteFunc = R"cpptemplate(
extern "C" US_ABI_EXPORT void DeleteInstance_{0}(ComponentInstance* componentInstance)
{
  delete componentInstance;
}
)cpptemplate";
      mStrStream << codegen::util::Substitute(deleteFunc, datamodel::GetComponentNameStr(componentInfo)) << "\n";
    }
  }

  const std::vector<std::string> mHeaderIncludes;
  const std::vector<ComponentInfo> mComponentInfos;
  std::stringstream mStrStream;
};
} // namespace codegen

#endif // COMPONENTCALLBACKGENERATOR_HPP
