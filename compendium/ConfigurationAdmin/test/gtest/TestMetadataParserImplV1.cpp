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
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/Framework.h"
#include "cppmicroservices/FrameworkEvent.h"
#include "cppmicroservices/FrameworkFactory.h"

#include "gtest/gtest.h"

#include "../../src/CMConstants.hpp"
#include "../../src/metadata/ConfigurationMetadata.hpp"
#include "../../src/metadata/MetadataParserFactory.hpp"
#include "../../src/metadata/MetadataParserImpl.hpp"
#include "Mocks.hpp"

using cppmicroservices::AnyMap;
using cppmicroservices::cmimpl::FakeLogger;
using cppmicroservices::cmimpl::MockLogger;
using cppmicroservices::cmimpl::CMConstants::CM_KEY;
using cppmicroservices::cmimpl::CMConstants::CM_VERSION;
using cppmicroservices::cmimpl::metadata::ConfigurationMetadata;
using cppmicroservices::cmimpl::metadata::MetadataParserFactory;
using cppmicroservices::cmimpl::metadata::MetadataParserImplV1;

#define str(s)  #s
#define xstr(s) str(s)

namespace
{

    namespace ManifestHelper
    {
        static cppmicroservices::Framework framework(cppmicroservices::FrameworkFactory().NewFramework());
        static cppmicroservices::Bundle thisBundle;

        static void
        StartFramework()
        {
            framework.Start();
            auto bundles = framework.GetBundleContext().GetBundles();
            auto thisBundleItr = std::find_if(bundles.begin(),
                                              bundles.end(),
                                              [](cppmicroservices::Bundle const& bundle)
                                              { return (xstr(US_BUNDLE_NAME) == bundle.GetSymbolicName()); });
            thisBundle = (thisBundleItr != bundles.end()) ? *thisBundleItr : cppmicroservices::Bundle();
            ASSERT_TRUE(static_cast<bool>(thisBundle)) << "bundle for pkgtest executable not found";
        }

        static void
        StopFramework()
        {
            framework.Stop();
            framework.WaitForStop(std::chrono::milliseconds::zero());
            thisBundle = nullptr;
        }

        static AnyMap const&
        GetTestManifest(std::string const& manifest_name)
        {
            AnyMap const& headers = thisBundle.GetHeaders();
            try
            {
                auto const& testMetadata = headers.AtCompoundKey("test_metadata." + manifest_name + "." + CM_KEY);
                return cppmicroservices::ref_any_cast<AnyMap>(testMetadata);
            }
            catch (std::out_of_range const& e)
            {
                std::cout << "Exception: " << e.what() << std::endl;
                for (auto const& kv : headers)
                {
                    std::cout << kv.first << " : " << kv.second.ToStringNoExcept() << std::endl;
                }
            }
            return headers;
        }
    } // namespace ManifestHelper

    // Test parsing representations of valid service descriptions
    class TestMetadataParserImplV1 : public ::testing::Test
    {
      protected:
        TestMetadataParserImplV1() : logger(std::make_shared<FakeLogger>()) {}
        ~TestMetadataParserImplV1() override = default;

      private:
        std::shared_ptr<FakeLogger> logger;

      public:
        static void
        SetUpTestCase()
        {
            ManifestHelper::StartFramework();
        }

        static void
        TearDownTestCase()
        {
            ManifestHelper::StopFramework();
        }

        std::shared_ptr<FakeLogger>
        GetLogger()
        {
            return logger;
        }
    };

    TEST_F(TestMetadataParserImplV1, ParseValidManifest)
    {
        auto metadataParser = MetadataParserFactory::Create(1, GetLogger());
        auto configurations
            = metadataParser->ParseAndGetConfigurationMetadata(ManifestHelper::GetTestManifest("manifest_json"));
        ASSERT_THAT(configurations, ::testing::SizeIs(1));
        auto const& configuration = configurations[0];
        ASSERT_EQ(configuration.pid, std::string("test"));
        ASSERT_THAT(configuration.properties, ::testing::SizeIs(3));
    }

    TEST_F(TestMetadataParserImplV1, ParseManifestEmptyProps)
    {
        auto metadataParser = MetadataParserFactory::Create(1, GetLogger());
        auto configurations
            = metadataParser->ParseAndGetConfigurationMetadata(ManifestHelper::GetTestManifest("manifest_empty_props"));
        ASSERT_THAT(configurations, ::testing::SizeIs(1));
        auto const& configuration = configurations[0];
        ASSERT_EQ(configuration.pid, std::string("test"));
        ASSERT_THAT(configuration.properties, ::testing::SizeIs(0));
    }

    TEST_F(TestMetadataParserImplV1, ParseManifestEmptyConfigurations)
    {
        auto metadataParser = MetadataParserFactory::Create(1, GetLogger());
        auto configurations
            = metadataParser->ParseAndGetConfigurationMetadata(ManifestHelper::GetTestManifest("manifest_empty_conf"));
        ASSERT_THAT(configurations, ::testing::SizeIs(0));
    }

    TEST_F(TestMetadataParserImplV1, ParseMultipleConfigurations)
    {
        auto metadataParser = MetadataParserFactory::Create(1, GetLogger());
        auto configurations
            = metadataParser->ParseAndGetConfigurationMetadata(ManifestHelper::GetTestManifest("manifest_mult_conf"));
        ASSERT_THAT(configurations, ::testing::SizeIs(2));

        auto configuration = configurations[0];
        ASSERT_EQ(configuration.pid, std::string("test"));
        ASSERT_THAT(configuration.properties, ::testing::SizeIs(1));

        configuration = configurations[1];
        ASSERT_EQ(configuration.pid, std::string("test2"));
        ASSERT_THAT(configuration.properties, ::testing::SizeIs(2));
    }

    struct MetadataInvalidManifestState
    {
        MetadataInvalidManifestState(std::string manifest, std::string errorOut)
            : manifestName(std::move(manifest))
            , errorOutput(std::move(errorOut))
        {
        }

        std::string manifestName;
        std::string errorOutput;

        friend std::ostream&
        operator<<(std::ostream& os, MetadataInvalidManifestState const& obj)
        {
            return os << "Manifest Name: " << obj.manifestName << " error output: " << obj.errorOutput << "\n";
        }
    };

    // Test failure modes where the exceptions are thrown
    class TestInvalidMetadata : public ::testing::TestWithParam<MetadataInvalidManifestState>
    {
      private:
        std::shared_ptr<FakeLogger> logger;

      protected:
        TestInvalidMetadata() : logger(std::make_shared<FakeLogger>()) {}

        ~TestInvalidMetadata() override = default;

      public:
        static void
        SetUpTestCase()
        {
            ManifestHelper::StartFramework();
        }

        static void
        TearDownTestCase()
        {
            ManifestHelper::StopFramework();
        }

        std::shared_ptr<FakeLogger>
        GetLogger()
        {
            return logger;
        }
    };

    TEST_P(TestInvalidMetadata, TestMetadataFailureModes)
    {
        auto const params = GetParam();
        EXPECT_THROW(
            {
                try
                {
                    const AnyMap& cm = ManifestHelper::GetTestManifest(params.manifestName);
                    if (0u == cm.count(CM_VERSION))
                    {
                        throw std::runtime_error(std::string("Metadata is missing mandatory '") + CM_VERSION
                                                 + "' property");
                    }
                    auto version = cppmicroservices::any_cast<int>(cm.at(CM_VERSION));
                    auto metadataParser = MetadataParserFactory::Create(version, GetLogger());
                    metadataParser->ParseAndGetConfigurationMetadata(cm);
                }
                catch (const std::exception& err)
                {
                    std::string exceptionStr { err.what() };
                    ASSERT_THAT(exceptionStr, ::testing::HasSubstr(params.errorOutput));
                    throw;
                }
            },
            std::exception);
    }

    INSTANTIATE_TEST_SUITE_P(
        FailureModes,
        TestInvalidMetadata,
        testing::Values(
            MetadataInvalidManifestState("manifest_illegal_ver", "Unsupported manifest file version '0'"),
            MetadataInvalidManifestState("manifest_missing_ver", "Metadata is missing mandatory 'version' property"),
            MetadataInvalidManifestState("manifest_illegal_ver2", "cppmicroservices::BadAnyCastException"),
            MetadataInvalidManifestState("manifest_illegal_ver3", "cppmicroservices::BadAnyCastException"),
            MetadataInvalidManifestState("manifest_no_conf", "Metadata is missing mandatory 'configurations' property"),
            MetadataInvalidManifestState("manifest_bad_conf", "cppmicroservices::BadAnyCastException"),
            MetadataInvalidManifestState("manifest_bad_arraytype", "cppmicroservices::BadAnyCastException")));

    // Test failure modes where the exceptions are eaten by metadataParser
    // and logged through the logger
    class TestInvalidMetadataThroughLogger : public ::testing::TestWithParam<MetadataInvalidManifestState>
    {
      public:
        static void
        SetUpTestCase()
        {
            ManifestHelper::StartFramework();
        }

        static void
        TearDownTestCase()
        {
            ManifestHelper::StopFramework();
        }
    };

    // We are only interested in the message logged by the metadataParser.
    class CustomLogger : public FakeLogger
    {
      public:
        std::string msg;
        using FakeLogger::Log;
        void
        Log(cppmicroservices::logservice::SeverityLevel /* level */, std::string const& message) override
        {
            msg = message;
        }
    };

    TEST_P(TestInvalidMetadataThroughLogger, TestMetadataFailureModes)
    {
        auto const params = GetParam();
        auto logger = std::make_shared<CustomLogger>();
        EXPECT_NO_THROW({
            const auto& cm = ManifestHelper::GetTestManifest(params.manifestName);
            auto version = cppmicroservices::any_cast<int>(cm.at(CM_VERSION));
            auto metadataParser = MetadataParserFactory::Create(version, logger);
            metadataParser->ParseAndGetConfigurationMetadata(cm);
        });
        EXPECT_THAT(logger->msg,
                    ::testing::HasSubstr("Could not load the configuration with index: " + params.errorOutput));
    }

    INSTANTIATE_TEST_SUITE_P(FailureModes,
                             TestInvalidMetadataThroughLogger,
                             testing::Values(MetadataInvalidManifestState("manifest_missing_pid", "0"),
                                             MetadataInvalidManifestState("manifest_missing_properties", "0"),
                                             MetadataInvalidManifestState("manifest_bad_pid", "0"),
                                             MetadataInvalidManifestState("manifest_bad_properties", "0"),
                                             MetadataInvalidManifestState("manifest_bad_index1", "1")));

} // namespace
