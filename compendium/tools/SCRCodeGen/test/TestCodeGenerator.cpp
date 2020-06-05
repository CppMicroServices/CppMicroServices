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

#include <regex>

#include "../ComponentCallbackGenerator.hpp"
#include "../ManifestParser.hpp"
#include "../ManifestParserFactory.hpp"
#include "ReferenceAutogenFiles.hpp"
#include "Util.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "json/json.h"

namespace codegen {
const std::string manifest_json = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "service": {
                       "scope": "singleton",
                       "interfaces": ["SpellCheck::ISpellCheckService"]
                       },
                       "references": [{
                         "name": "dictionary",
                         "interface": "DictionaryService::IDictionaryService"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_dyn = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "inject-references" : true,
                       "service": {
                       "scope": "SINGLETON", //case-insensitive
                       "interfaces": ["SpellCheck::ISpellCheckService"]
                       },
                       "references": [{
                         "name": "dictionary",
                         "interface": "DictionaryService::IDictionaryService",
                         "policy": "dynamic"
                       }]
                       }]
            }
  }
  )manifest";

const std::string manifest_mult_comp = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                      "implementation-class": "Foo::Impl1",
                         "service": {
                         "interfaces": ["Foo::Interface"]
                         }
                       },
                       {"implementation-class": "Foo::Impl2",
                         "service": {
                         "interfaces": ["Foo::Interface"]
                         }
                       }]
            }
  }
  )manifest";

const std::string manifest_mult_comp_same_impl = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                      "implementation-class": "Foo::Impl1",
                      "name": "FooImpl1",
                         "service": {
                         "interfaces": ["Foo::Interface"]
                         }
                       },
                       {"implementation-class": "Foo::Impl1",
                        "name": "FooImpl2",
                         "service": {
                         "interfaces": ["Foo::Interface"]
                         }
                       }]
            }
  }
  )manifest";

const std::string manifest_no_scr = R"manifest(
  {
  }
  )manifest";

const std::string manifest_empty_scr = R"manifest(
  {
    "scr" : ""
  }
  )manifest";

const std::string manifest_illegal_scr = R"manifest(
  {
    "scr" : 911
  }
  )manifest";

const std::string manifest_illegal_ver = R"manifest(
  {
    "scr" : { "version" : 0,
              "components": [{
                       }]
            }
  }
  )manifest";

const std::string manifest_missing_ver = R"manifest(
  {
    "scr" : {"components": [{
                      "implementation-class": "Foo::Impl1",
                         "service": {
                         "interfaces": ["Foo::Interface"]
                         }
                       }]
            }
  }
  )manifest";

const std::string manifest_illegal_ver2 = R"manifest(
  {
    "scr" : { "version" : "",
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl"
           }]
           }
  }
  )manifest";

const std::string manifest_illegal_ver3 = R"manifest(
  {
    "scr" : { "version" : "one",
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl"
           }]
           }
  }
  )manifest";

const std::string manifest_illegal_ver4 = R"manifest(
  {
    "scr" : { "version" : ,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl"
           }]
           }
  }
  )manifest";

const std::string manifest_dup_keys = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl"
                       }]
           },
    "scr" : "duplicated"
  }
  )manifest";

const std::string manifest_no_comp = R"manifest(
  {
    "scr" : { "version" : 1
            }
  }
  )manifest";

const std::string manifest_no_impl_class = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "service": {
                       "scope": "singleton",
                       "interfaces": ["SpellCheck::ISpellCheckService"]
                       },
                       "references": [{
                         "name": "dictionary",
                         "interface": "DictionaryService::IDictionaryService"
                       }]
                       }]
            }
  }
  )manifest";

const std::string manifest_no_ref_name = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "service": {
                       "scope": "singleton",
                       "interfaces": ["SpellCheck::ISpellCheckService"]
                       },
                       "references": [{
                         "interface": "DictionaryService::IDictionaryService"
                       }]
                       }]
            }
  }
  )manifest";

const std::string manifest_no_ref_interface = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "service": {
                       "scope": "singleton",
                       "interfaces": ["SpellCheck::ISpellCheckService"]
                       },
                       "references": [{
                         "name": "dictionary"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_illegal_service = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "service": 911
                       }]
           }
  }
  )manifest";

const std::string manifest_no_interfaces = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "activate" : "Activate",
                       "deactivate" : "Deactivate",
                       "service": {
                       "scope": "singleton"
                       },
                       "references": [{
                         "name": "dictionary",
                         "interface": "DictionaryService::IDictionaryService"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_empty_interfaces_arr = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "service": {
                       "scope": "singleton",
                       "interfaces": []
                       }
                       }]
           }
  }
  )manifest";

const std::string manifest_empty_impl_class = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": ""
                       }]
            }
  }
  )manifest";

const std::string manifest_illegal_inject_refs = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "Foo",
                       "inject-references": "true"
                       }]
            }
  }
  )manifest";

const std::string manifest_illegal_inject_refs2 = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "Foo",
                       "inject-references": {}
                       }]
            }
  }
  )manifest";

const std::string manifest_empty_ref_name = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": [{
                         "name": "",
                         "interface": "Bar"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_illegal_ref_name = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": [{
                         "name": 729,
                         "interface": "Bar"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_duplicate_ref_name = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": [{
                         "name": "foo",
                         "interface": "Bar"
                       },
                       {
                         "name": "foo",
                         "interface": "Bar"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_illegal_comp = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": []
            }
  }
  )manifest";

const std::string manifest_empty_ref_interface = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": [{
                         "name": "Foo",
                         "interface": ""
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_illegal_ref_interface = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": [{
                         "name": "Foo",
                         "interface": 911
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_dup_ref_interface = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": [{
                         "name": "Foo",
                         "interface": "Foo::Bar",
                         "interface": "Foo::Baz"
                       }]
                       }]
           }
  }
  )manifest";

const std::string manifest_illegal_scope = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "service": {
                       "scope": "global",
                       "interfaces": ["SpellCheck::ISpellCheckService"]
                       }
                       }]
           }
  }
  )manifest";

const std::string manifest_illegal_ref = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                       "implementation-class": "DSSpellCheck::SpellCheckImpl",
                       "references": "illegal"
                       }]
            }
  }
  )manifest";

const std::string manifest_illegal_interfaces = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                      "implementation-class": "Foo::Impl1",
                         "service": {
                         "interfaces": [1, 2]
                         }
                       }
                       ]
            }
  }
  )manifest";

const std::string manifest_empty_interfaces_string = R"manifest(
  {
    "scr" : { "version" : 1,
              "components": [{
                      "implementation-class": "Foo::Impl1",
                         "service": {
                         "interfaces": ["", "valid", ""]
                         }
                       }
                       ]
            }
  }
  )manifest";

auto GetManifestSCRData(const std::string& content)
{
  std::istringstream istrstream(content);
  auto root = util::ParseManifestOrThrow(istrstream);
  return util::JsonValueValidator(root, "scr", Json::ValueType::objectValue)();
};

// For the manifest specified in the member manifest and headers specified in headers,
// we expect the output generated by the code-generator to be exactly referenceOutput.
struct CodegenValidManifestState
{
  CodegenValidManifestState(std::string _manifest,
                            std::vector<std::string> _headers,
                            std::string _referenceOutput)
    : manifest(std::move(_manifest))
    , headers(std::move(_headers))
    , referenceOutput(std::move(_referenceOutput))
  {}

  std::string manifest;
  std::vector<std::string> headers;
  std::string referenceOutput;

  friend std::ostream& operator<<(std::ostream& os,
                                  const CodegenValidManifestState& obj)
  {
    os << "Manifest: " << obj.manifest << "\nHeaders: ";
    std::for_each(obj.headers.begin(),
                  obj.headers.end(),
                  [&os](const std::string& header) { os << header << "\n  "; });
    return os << "\nReference Output: " << obj.referenceOutput << "\n";
  }
};

class ValidCodegenTest
  : public ::testing::TestWithParam<CodegenValidManifestState>
{};

TEST_P(ValidCodegenTest, TestCodegenFunctionality)
{
  CodegenValidManifestState vcs = GetParam();
  auto scr = GetManifestSCRData(vcs.manifest);
  auto version =
    util::JsonValueValidator(scr, "version", Json::ValueType::intValue)();

  auto manifestParser = ManifestParserFactory::Create(version.asInt());
  auto componentInfos = manifestParser->ParseAndGetComponentInfos(scr);
  ComponentCallbackGenerator compGen(vcs.headers, componentInfos);
  EXPECT_EQ(compGen.GetString(), vcs.referenceOutput);
}

INSTANTIATE_TEST_SUITE_P(
  SuccessModes,
  ValidCodegenTest,
  testing::Values(
    // valid manifest
    CodegenValidManifestState(manifest_json,
                              { "SpellCheckerImpl.hpp" },
                              REF_SRC),
    // valid manifest with dynamic policy
    CodegenValidManifestState(manifest_dyn,
                              { "SpellCheckerImpl.hpp" },
                              REF_SRC_DYN),
    // valid manifest with multiple components
    CodegenValidManifestState(manifest_mult_comp,
                              { "A.hpp", "B.hpp", "C.hpp" },
                              REF_MULT_COMPS),
    // valid manifest with multiple components of the same implementation class
    CodegenValidManifestState(manifest_mult_comp_same_impl,
                              { "A.hpp", "B.hpp", "C.hpp" },
                              REF_MULT_COMPS_SAME_IMPL)));

// For the manifest specified in the member manifest, we expect the exception message
// output by the code-generator to be exactly errorOutput.
// Instead, if we expect the errorOutput to be contained in the generated error message,
// we set isPartial = true. (This is useful when we don't want to specify really long error messages)
struct CodegenInvalidManifestState
{
  CodegenInvalidManifestState(std::string _manifest,
                              std::string _errorOutput,
                              bool _isPartial = false)
    : manifest(std::move(_manifest))
    , errorOutput(std::move(_errorOutput))
    , isPartial(_isPartial)
  {}

  std::string manifest;
  std::string errorOutput;
  bool isPartial;

  friend std::ostream& operator<<(std::ostream& os,
                                  const CodegenInvalidManifestState& obj)
  {
    return os << "Manifest: " << obj.manifest
              << " Error output: " << obj.errorOutput
              << "  Perform partial match: " << (obj.isPartial ? "Yes" : "No")
              << "\n";
  }
};

class InvalidCodegenTest
  : public ::testing::TestWithParam<CodegenInvalidManifestState>
{};

// Test failure modes where mandatory manifest names are missing or empty
TEST_P(InvalidCodegenTest, TestCodegenFailureModes)
{
  CodegenInvalidManifestState ics = GetParam();
  try {
    auto scr = GetManifestSCRData(ics.manifest);
    auto version =
      util::JsonValueValidator(scr, "version", Json::ValueType::intValue)();
    auto manifestParser = ManifestParserFactory::Create(version.asInt());
    manifestParser->ParseAndGetComponentInfos(scr);
    FAIL() << "This failure suggests that parsing has succeeded. "
              "Shouldn't happen for failure mode tests";
  } catch (const std::exception& err) {
    if (!ics.isPartial) {
      ASSERT_STREQ(ics.errorOutput.c_str(), err.what());
    } else {
      const std::string regex = "(" + ics.errorOutput + ")";
      ASSERT_TRUE(std::regex_search(err.what(), std::regex(regex)));
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
  FailureModes,
  InvalidCodegenTest,
  testing::Values(
    CodegenInvalidManifestState(
      manifest_no_scr,
      "Mandatory name 'scr' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_empty_scr,
      "Invalid value for the name 'scr'. Expected non-empty JSON object i.e. "
      "collection of name/value pairs"),
    CodegenInvalidManifestState(
      manifest_illegal_scr,
      "Invalid value for the name 'scr'. Expected non-empty JSON object i.e. "
      "collection of name/value pairs"),
    // We test the duplicate names only twice because the check is done by the JSON parser
    // and we trust its validation.
    // We have a test point for a duplicate name at the root and in the interior
    CodegenInvalidManifestState(manifest_dup_keys,
                                "Duplicate key: 'scr'",
                                /*isPartial=*/true),
    CodegenInvalidManifestState(manifest_illegal_ver,
                                "Unsupported manifest file version '0'"),
    CodegenInvalidManifestState(
      manifest_illegal_ver2,
      "Invalid value for the name 'version'. Expected int"),
    CodegenInvalidManifestState(
      manifest_illegal_ver3,
      "Invalid value for the name 'version'. Expected int"),
    CodegenInvalidManifestState(manifest_illegal_ver4,
                                "Syntax error: value, object or array expected",
                                /*isPartial=*/true),
    CodegenInvalidManifestState(
      manifest_missing_ver,
      "Mandatory name 'version' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_no_comp,
      "Mandatory name 'components' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_illegal_comp,
      "Invalid value for the name 'components'. Expected non-empty array"),
    CodegenInvalidManifestState(
      manifest_no_impl_class,
      "Mandatory name 'implementation-class' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_empty_impl_class,
      "Invalid value for the name 'implementation-class'. Expected non-empty "
      "string"),
    CodegenInvalidManifestState(
      manifest_no_ref_name,
      "Mandatory name 'name' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_empty_ref_name,
      "Invalid value for the name 'name'. Expected non-empty string"),
    CodegenInvalidManifestState(
      manifest_illegal_ref_name,
      "Invalid value for the name 'name'. Expected non-empty string"),
    CodegenInvalidManifestState(
        manifest_duplicate_ref_name,
        "Duplicate service reference names found. Reference names must be unique. Duplicate names: foo "),
    CodegenInvalidManifestState(
      manifest_no_ref_interface,
      "Mandatory name 'interface' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_empty_ref_interface,
      "Invalid value for the name 'interface'. Expected non-empty string"),
    CodegenInvalidManifestState(
      manifest_illegal_ref_interface,
      "Invalid value for the name 'interface'. Expected non-empty string"),
    CodegenInvalidManifestState(manifest_dup_ref_interface,
                                "Duplicate key: 'interface'",
                                /*isPartial=*/true),
    CodegenInvalidManifestState(
      manifest_illegal_service,
      "Invalid value for the name 'service'. Expected non-empty JSON object "
      "i.e. collection of name/value pairs"),
    CodegenInvalidManifestState(
      manifest_no_interfaces,
      "Mandatory name 'interfaces' missing from the manifest"),
    CodegenInvalidManifestState(
      manifest_empty_interfaces_arr,
      "Invalid value for the name 'interfaces'. Expected non-empty array"),
    CodegenInvalidManifestState(manifest_illegal_interfaces,
                                "Invalid array value for the name "
                                "'interfaces'. Expected non-empty string"),
    CodegenInvalidManifestState(manifest_empty_interfaces_string,
                                "Invalid array value for the name "
                                "'interfaces'. Expected non-empty string"),
    CodegenInvalidManifestState(
      manifest_illegal_inject_refs,
      "Invalid value for the name 'inject-references'. Expected boolean"),
    CodegenInvalidManifestState(
      manifest_illegal_inject_refs2,
      "Invalid value for the name 'inject-references'. Expected boolean"),
    CodegenInvalidManifestState(
      manifest_illegal_scope,
      "Invalid value 'global' for the name 'scope'. The valid choices are : "
      "[singleton, bundle, prototype]"),
    CodegenInvalidManifestState(
      manifest_illegal_ref,
      "Invalid value for the name 'references'. Expected non-empty array")));

} // namespace codegen
