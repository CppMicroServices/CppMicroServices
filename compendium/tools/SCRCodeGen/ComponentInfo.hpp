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
#ifndef COMPONENTINFO_HPP
#define COMPONENTINFO_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <stdexcept>

namespace codegen
{
    namespace datamodel
    {

        struct ReferenceInfo
        {
            std::string name;
            std::string interface;
            std::string cardinality;
            std::string policy;
            std::string policy_option;
            std::string target;
            std::string scope;
        };

        struct ServiceInfo
        {
            std::string scope;
            std::vector<std::string> interfaces;
        };

        struct ComponentInfo
        {
            std::string name;
            std::string implClassName;
            std::string configurationPolicy;
            ServiceInfo service;
            std::vector<ReferenceInfo> references;
            static const std::string CONFIG_POLICY_IGNORE;
            static const std::string CONFIG_POLICY_REQUIRE;
            static const std::string CONFIG_POLICY_OPTIONAL;

            // check if the specific reference is injected
            // true if either
            //     injectAllRefs is true or
            //     this one is injected
            bool injectRef(std::string refName) const {
                return injectAllRefs || injectedRefs.find(refName) != injectedRefs.end();
            }


            // instantiate the set of injected refs or the 'all' bool
            void setInjectReferences(bool allRefs, std::unordered_set<std::string> refNames = {}){
                // if the set of refNames is empty, set the injectAll param
                if (refNames.empty()){
                    injectAllRefs = allRefs;
                    return;
                }

                // set injectedRefs
                injectedRefs = refNames;
                injectAllRefs = allRefs;
            }

            // ensure that all references in the set of injectedRefs are named references of the component
            void validateInjectedRefNames(){
                std::unordered_set<std::string> allRefNames;
                for (auto const& ref : references){
                    allRefNames.insert(ref.name);
                }

                for (auto const & injectedRef : injectedRefs){
                    if (allRefNames.find(injectedRef) == allRefNames.end()){
                        throw std::runtime_error("The injected reference with name '"
                                                 + injectedRef + 
                                                 "' does not match any of this service's references");
                    }
                }
            }
          private: 
            bool injectAllRefs = true;
            std::unordered_set<std::string> injectedRefs;
        };

        // These functions return the string representations for the data model
        // that's understood by the code-generator
        std::string GetComponentNameStr(ComponentInfo const& compInfo);
        std::string GetServiceInterfacesStr(ServiceInfo const& compInfo);
        std::string GetCtorInjectedRefTypes(ComponentInfo const& compInfo);
        std::string GetCtorInjectedRefNames(ComponentInfo const& compInfo);
        std::string GetReferenceBinderStr(ReferenceInfo const& ref);
        std::string GetCtorInjectedRefParameters(ComponentInfo const& compInfo);

    } // namespace datamodel
} // namespace codegen

#endif // COMPONENTINFO_HPP
