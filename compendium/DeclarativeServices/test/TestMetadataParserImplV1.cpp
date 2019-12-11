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
#include "Mocks.hpp"
#include "cppmicroservices/servicecomponent/ComponentConstants.hpp"
#include "../src/metadata/MetadataParserFactory.hpp"
#include "../src/metadata/MetadataParserImpl.hpp"
#include "../src/metadata/ReferenceMetadata.hpp"
#include "../src/metadata/Util.hpp"
#include "gtest/gtest.h"
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/FrameworkFactory.h>

#include <iostream>

using cppmicroservices::Any;
using cppmicroservices::AnyMap;
using cppmicroservices::scrimpl::FakeLogger;
using cppmicroservices::scrimpl::MockLogger;
using cppmicroservices::scrimpl::metadata::ComponentMetadata;
using cppmicroservices::scrimpl::metadata::MetadataParserFactory;
using cppmicroservices::scrimpl::metadata::MetadataParserImplV1;
using cppmicroservices::scrimpl::util::ObjectValidator;
using cppmicroservices::service::component::ComponentConstants::
  SERVICE_COMPONENT;

#define str(s) #s
#define xstr(s) str(s)

namespace {
namespace ManifestHelper {

AnyMap GetTestManifest(const std::string& manifest_name)
{
  cppmicroservices::Framework framework(
    cppmicroservices::FrameworkFactory().NewFramework());
  framework.Start();
  auto bundles = framework.GetBundleContext().GetBundles();
  auto thisBundleItr = std::find_if(
    bundles.begin(), bundles.end(), [](const cppmicroservices::Bundle& bundle) {
      return (bundle.GetSymbolicName() == xstr(US_BUNDLE_NAME));
    });
  auto thisBundle = (thisBundleItr != bundles.end())
                      ? *thisBundleItr
                      : cppmicroservices::Bundle();
  const AnyMap& headers = thisBundle.GetHeaders();
  try {
    auto const& testMetadata = headers.AtCompoundKey(
      "test_metadata." + manifest_name + "." + SERVICE_COMPONENT);
    return cppmicroservices::ref_any_cast<AnyMap>(testMetadata);
  } catch (const std::out_of_range& e) {
    std::cout << "Exception: " << e.what() << std::endl;
    for (auto const& kv : headers) {
      std::cout << kv.first << " : " << kv.second.ToStringNoExcept()
                << std::endl;
    }
  }
  AnyMap manifestData{ headers };
  framework.Stop();
  framework.WaitForStop(std::chrono::milliseconds::zero());
  return manifestData;
}
}

// Test parsing representations of valid service descriptions
class MetadataParserImplV1Test : public ::testing::Test
{
protected:
  MetadataParserImplV1Test()
    : logger(std::make_shared<FakeLogger>())
  {}
  ~MetadataParserImplV1Test() = default;
  std::shared_ptr<FakeLogger> logger;

public:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}
  std::shared_ptr<FakeLogger> GetLogger() { return logger; }
};

TEST_F(MetadataParserImplV1Test, ParseValidManifest)
{
  auto metadataparser = MetadataParserFactory::Create(1, GetLogger());
  auto components = metadataparser->ParseAndGetComponentsMetadata(
    ManifestHelper::GetTestManifest("manifest_json"));
  const auto component = components[0];
  ASSERT_EQ(component->activateMethodName, std::string("Activate"));
  ASSERT_EQ(component->deactivateMethodName, std::string("Deactivate"));
  ASSERT_EQ(component->modifiedMethodName, std::string("Modified"));
  ASSERT_EQ(component->immediate, false);
  ASSERT_EQ(component->enabled, true);
  ASSERT_EQ(component->name, "DSSpellCheck::SpellCheckImpl");
  ASSERT_EQ(component->implClassName, "DSSpellCheck::SpellCheckImpl");
  ASSERT_THAT(component->properties, ::testing::SizeIs(0));
}

TEST_F(MetadataParserImplV1Test, ParseDynManifest)
{
  auto metadataparser = MetadataParserFactory::Create(1, GetLogger());
  auto components = metadataparser->ParseAndGetComponentsMetadata(
    ManifestHelper::GetTestManifest("manifest_dyn"));
  const auto component = components[0];
  ASSERT_EQ(component->activateMethodName, std::string("Activate"));
  ASSERT_EQ(component->deactivateMethodName, std::string("Deactivate"));
  ASSERT_EQ(component->modifiedMethodName, std::string("Modified"));
  ASSERT_EQ(component->immediate, false);
  ASSERT_EQ(component->enabled, false);
  ASSERT_EQ(component->name, "DSSpellCheck::SpellCheckImpl");
  ASSERT_EQ(component->implClassName, "DSSpellCheck::SpellCheckImpl");
  ASSERT_THAT(component->properties, ::testing::SizeIs(2));
  ASSERT_EQ(
    cppmicroservices::any_cast<std::string>(component->properties["foo"]),
    std::string("bar"));
  ASSERT_EQ(cppmicroservices::any_cast<int>(component->properties["num_comps"]),
            42);
}

TEST_F(MetadataParserImplV1Test, ParseMultComps)
{
  auto metadataparser = MetadataParserFactory::Create(1, GetLogger());
  auto components = metadataparser->ParseAndGetComponentsMetadata(
    ManifestHelper::GetTestManifest("manifest_mult_comp"));
  ASSERT_THAT(components, ::testing::SizeIs(2));

  auto component = components[0];
  ASSERT_EQ(component->activateMethodName, std::string("Activate"));
  ASSERT_EQ(component->deactivateMethodName, std::string("Deactivate"));
  ASSERT_EQ(component->modifiedMethodName, std::string("Modified"));
  ASSERT_EQ(component->immediate, true);
  ASSERT_EQ(component->enabled, true);
  ASSERT_EQ(component->name, "Foo::Impl1");
  ASSERT_EQ(component->implClassName, "Foo::Impl1");

  component = components[1];
  ASSERT_EQ(component->activateMethodName, std::string("Activate"));
  ASSERT_EQ(component->deactivateMethodName, std::string("Deactivate"));
  ASSERT_EQ(component->modifiedMethodName, std::string("Modified"));
  ASSERT_EQ(component->immediate, true);
  ASSERT_EQ(component->enabled, true);
  ASSERT_EQ(component->name, "Foo::Impl2");
  ASSERT_EQ(component->implClassName, "Foo::Impl2");
}

// For the SCR map specified in "scr", we expect the exception message
// output by the Metadata Parser to be exactly errorOutput.
// Instead, if we expect the errorOutput to be contained in the generated error message,
// we set isPartial = true. (This mode is useful when we don't want to specify really
// long error messages)
struct MetadataInvalidManifestState
{
  MetadataInvalidManifestState(std::string manifestName,
                               std::string _errorOutput,
                               bool _isPartial = false)
    : manifestName(std::move(manifestName))
    , errorOutput(std::move(_errorOutput))
    , isPartial(_isPartial)
  {}

  std::string manifestName;
  std::string errorOutput;
  bool isPartial;

  friend std::ostream& operator<<(std::ostream& os,
                                  const MetadataInvalidManifestState& obj)
  {
    return os << "Manifest Name: " << obj.manifestName
              << " error output: " << obj.errorOutput
              << "  Perform partial match: " << (obj.isPartial ? "Yes" : "No")
              << "\n";
  }
};

// Test failure modes where the exceptions are thrown
class InvalidMetadataTest
  : public ::testing::TestWithParam<MetadataInvalidManifestState>
{
protected:
  std::shared_ptr<FakeLogger> logger;

  InvalidMetadataTest()
    : logger(std::make_shared<FakeLogger>())
  {}

  ~InvalidMetadataTest() = default;

public:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}

  std::shared_ptr<FakeLogger> GetLogger() { return logger; }
};

TEST_P(InvalidMetadataTest, TestMetadataFailureModes)
{
  MetadataInvalidManifestState ims = GetParam();
  try {
    const AnyMap& scr = ManifestHelper::GetTestManifest(ims.manifestName);
    auto version = ObjectValidator(scr, "version").GetValue<int>();
    auto metadataparser = MetadataParserFactory::Create(version, GetLogger());
    metadataparser->ParseAndGetComponentsMetadata(scr);
    FAIL() << "This failure suggests that parsing has succeeded. "
              "Shouldn't happen for failure mode tests";
  } catch (const std::exception& err) {
    std::string exceptionStr{ err.what() };
    if (!ims.isPartial) {
      ASSERT_THAT(exceptionStr, ::testing::StrEq(ims.errorOutput));
    } else {
      ASSERT_THAT(exceptionStr, ::testing::HasSubstr(ims.errorOutput));
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
  FailureModes,
  InvalidMetadataTest,
  testing::Values(
    MetadataInvalidManifestState("manifest_illegal_ver",
                                 "Unsupported manifest file version '0'"),
    MetadataInvalidManifestState("manifest_missing_ver",
                                 "Missing key 'version' in the manifest."),
    MetadataInvalidManifestState("manifest_illegal_ver2",
                                 "cppmicroservices::BadAnyCastException",
                                 /*isPartial=*/true),
    MetadataInvalidManifestState("manifest_illegal_ver3",
                                 "cppmicroservices::BadAnyCastException",
                                 /*isPartial=*/true),
    MetadataInvalidManifestState(
      "manifest_illegal_comp",
      "Value for the name 'components' cannot be empty.",
      /*isPartial=*/true),
    MetadataInvalidManifestState("manifest_no_comp",
                                 "Missing key 'components' in the manifest.")));

// Test failure modes where the exceptions are eaten by MetadataParser
// and logged through the logger
class InvalidMetadataTestThroughLogger
  : public ::testing::TestWithParam<MetadataInvalidManifestState>
{
public:
  static void SetUpTestCase() {}

  static void TearDownTestCase() {}
};

// We are only interested in the message logged by the MetadataParser.
class CustomLogger : public FakeLogger
{
public:
  std::string msg;
  using FakeLogger::Log;
  void Log(cppmicroservices::logservice::SeverityLevel,
           const std::string& message) override
  {
    msg = message;
  }
};

TEST_P(InvalidMetadataTestThroughLogger, TestMetadataFailureModes)
{
  MetadataInvalidManifestState imsl = GetParam();
  const AnyMap& scr = ManifestHelper::GetTestManifest(imsl.manifestName);
  auto version = ObjectValidator(scr, "version").GetValue<int>();
  auto logger = std::make_shared<CustomLogger>();
  auto metadataparser = MetadataParserFactory::Create(version, logger);
  metadataparser->ParseAndGetComponentsMetadata(scr);
  // MetadataInvalidManifestState.isPartial is irrelevant here. We always do substring compare
  EXPECT_THAT(logger->msg, ::testing::HasSubstr(imsl.errorOutput));
}

INSTANTIATE_TEST_SUITE_P(
  FailureModes,
  InvalidMetadataTestThroughLogger,
  testing::Values(
    MetadataInvalidManifestState("manifest_badcomp_index_1",
                                 "Could not load the component with index: 1"),
    MetadataInvalidManifestState(
      "manifest_illegal_immediate",
      "Invalid value specified for the name 'immediate'. Could not load the "
      "component with index: 0"),
    MetadataInvalidManifestState(
      "manifest_no_impl_class",
      "Missing key 'implementation-class' in the manifest. Could not load the "
      "component with index: 0"),
    MetadataInvalidManifestState("manifest_no_ref_name",
                                 "Missing key 'name' in the manifest. Could "
                                 "not load the component with index: 0"),
    MetadataInvalidManifestState("manifest_no_ref_interface",
                                 "Missing key 'interface' in the manifest. "
                                 "Could not load the component with index: 0"),
    MetadataInvalidManifestState("manifest_illegal_service",
                                 "cppmicroservices::BadAnyCastException"),
    MetadataInvalidManifestState("manifest_no_interfaces",
                                 "Missing key 'interfaces' in the manifest. "
                                 "Could not load the component with index: 0"),
    MetadataInvalidManifestState(
      "manifest_empty_interfaces_arr",
      "Value for the name 'interfaces' cannot be empty."),
    MetadataInvalidManifestState(
      "manifest_empty_impl_class",
      "Value for the name 'implementation-class' cannot be empty."),
    MetadataInvalidManifestState("manifest_illegal_ref_arr",
                                 "Value cannot be empty."),
    MetadataInvalidManifestState(
      "manifest_empty_ref",
      "Value for the name 'references' cannot be empty."),
    MetadataInvalidManifestState("manifest_empty_ref_map",
                                 "Value cannot be empty."),
    MetadataInvalidManifestState("manifest_empty_ref_name",
                                 "Value for the name 'name' cannot be empty."),
    MetadataInvalidManifestState("manifest_illegal_ref_name",
                                 "cppmicroservices::BadAnyCastException:"),
    MetadataInvalidManifestState(
      "manifest_empty_ref_interface",
      "Value for the name 'interface' cannot be empty."),
    MetadataInvalidManifestState(
      "manifest_illegal_ref_interface",
      "Unexpected type for the name 'interface'. Exception: "
      "cppmicroservices::BadAnyCastException: "),
    MetadataInvalidManifestState(
      "manifest_illegal_scope",
      "Invalid value 'global'. The valid choices are : [bundle, prototype, "
      "singleton]. Could not load the component with index: 0"),
    MetadataInvalidManifestState(
      "manifest_illegal_ref",
      "Unexpected type for the name 'references'. Exception: "
      "cppmicroservices::BadAnyCastException"),
    MetadataInvalidManifestState("manifest_empty_interfaces_string",
                                 "Value cannot be empty.")));

//  TEST(GetComponentImplClassSymbolNameTest, TestRegexErrors)
//  {
//    auto compMetadata = std::make_shared<ComponentMetadata>();
//    compMetadata->name = "foo::bar";
//    ASSERT_NO_THROW(
//      ASSERT_THAT("foo_bar", ::testing::StrEq(GetComponentImplClassSymbolName(compMetadata)));
//    );
//    compMetadata->name = "";
//    ASSERT_NO_THROW(
//      ASSERT_THAT("", ::testing::StrEq(GetComponentImplClassSymbolName(compMetadata)));
//    );
//    compMetadata->name = "::::::::::";
//    ASSERT_NO_THROW(
//      ASSERT_THAT("_____", ::testing::StrEq(GetComponentImplClassSymbolName(compMetadata)));
//    );
//  }

} // namespace
