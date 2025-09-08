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

#if defined(USING_GTEST)
#    include "gtest/gtest_prod.h"
#else
#    define FRIEND_TEST(x, y)
#endif
#include "ComponentInfo.hpp"
#include "Util.hpp"

using codegen::datamodel::ComponentInfo;

namespace codegen
{

    class ComponentCallbackGenerator
    {
      public:
        ComponentCallbackGenerator(std::vector<std::string> const& includeHeaders,
                                   std::vector<ComponentInfo> const& componentInfos)
            : mHeaderIncludes(includeHeaders)
            , mComponentInfos(componentInfos)
            , mStrStream()
        {
            Substitute();
        }

        std::string
        GetString() const
        {
            return mStrStream.str();
        }

      private:
        FRIEND_TEST(CodeGenTest, TestSubstitution);
        FRIEND_TEST(CodeGenTest, TestSubstitutionDynamic);
        FRIEND_TEST(CodeGenTest, TestMultipleComponents);

        void
        Substitute()
        {
            SubstituteHeader();
            SubstituteBody();
        }

        void
        SubstituteHeader()
        {
            mStrStream << std::endl
                       << R"(#include <vector>)" << std::endl
                       << R"(#include <cppmicroservices/ServiceInterface.h>)" << std::endl
                       << R"(#include "cppmicroservices/servicecomponent/detail/ComponentInstanceImpl.hpp")"
                       << std::endl;

            for (auto const& header : mHeaderIncludes)
            {
                mStrStream << util::Substitute(R"(#include "{0}")", header) << std::endl;
            }
            mStrStream << std::endl
                       << "namespace sc = cppmicroservices::service::component;" << std::endl
                       << "namespace scd = cppmicroservices::service::component::detail;" << std::endl;
        }

        void
        SubstituteBody()
        {
            for (auto const& componentInfo : mComponentInfos)
            {
                // Generate the factory function for creating each component
                mStrStream << std::endl
                           << util::Substitute(R"(extern "C" US_ABI_EXPORT scd::ComponentInstance* NewInstance_{0}())",
                                               datamodel::GetComponentNameStr(componentInfo))
                           << std::endl
                           << "{" << std::endl;

                auto isReferencesEmpty = componentInfo.references.empty();
                if (!isReferencesEmpty)
                {
                    mStrStream << util::Substitute("  std::vector<std::shared_ptr<scd::Binder<{0}>>> binders;",
                                                   componentInfo.implClassName)
                               << std::endl;
                }

                for (auto const& ref : componentInfo.references)
                {
                    if (componentInfo.injectRef(ref.name) && ref.policy == "dynamic")
                    {
                        mStrStream << "  binders.push_back("
                                   << util::Substitute(
                                          datamodel::GetReferenceBinderStr(ref),
                                          componentInfo.implClassName)
                                   << ");" << std::endl;
                    }
                }

                mStrStream
                    << "  scd::ComponentInstance* componentInstance = new (std::nothrow) scd::ComponentInstanceImpl<"
                    << componentInfo.implClassName << ", std::tuple<"
                    << datamodel::GetServiceInterfacesStr(componentInfo.service) << ">";

                if (isReferencesEmpty)
                {
                    mStrStream << ">();";
                }
                else
                {

                    mStrStream << datamodel::GetCtorInjectedRefParameters(componentInfo) << ">("
                               << datamodel::GetCtorInjectedRefNames(componentInfo) << ", binders"
                               << ");";
                }

                mStrStream << std::endl << std::endl << "  return componentInstance;" << std::endl << "}" << std::endl;

                // Create deleter function for each component.
                mStrStream
                    << std::endl
                    << util::Substitute(
                           R"(extern "C" US_ABI_EXPORT void DeleteInstance_{0}(scd::ComponentInstance* componentInstance))",
                           datamodel::GetComponentNameStr(componentInfo))
                    << std::endl
                    << "{" << std::endl
                    << "  delete componentInstance;" << std::endl
                    << "}" << std::endl
                    << std::endl;
            }
        }

        std::vector<std::string> const mHeaderIncludes;
        std::vector<ComponentInfo> const mComponentInfos;
        std::stringstream mStrStream;
    };

} // namespace codegen
#endif
