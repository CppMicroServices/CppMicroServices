#include "CNTestingConfig.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace fs = std::filesystem;

/**
 * Test fixture for namespace replacement functionality
 * Tests various aspects of namespace replacement in different file types
 * and scenarios
 */
class NamespaceReplacerTest : public ::testing::Test
{
  protected:
    static constexpr const char* NEW_NAMESPACE = "newnamespace";
    fs::path tempDir; // Temporary directory for test files

    // Creates a temporary directory for each test
    void
    SetUp() override
    {
        tempDir = fs::temp_directory_path() / "change_namespace_test";
        fs::create_directories(tempDir);
    }

    // Cleans up the temporary directory after each test
    void
    TearDown() override
    {
        fs::remove_all(tempDir);
    }

    // Helper struct to encapsulate test parameters
    struct ReplacerParams
    {
        std::string input;
        std::string newNamespace;
        std::string fileType = "cpp";
    };

    /**
     * Creates test file and runs the namespace replacer
     * @param params Test parameters including input content and namespace
     * @return String containing processed output
     */
    std::string
    runReplacer(ReplacerParams const& params)
    {
        const fs::path inputFile = createInputFile(params);
        const fs::path outputDir = tempDir / "output";
        fs::create_directories(outputDir);

        runExecutable(tempDir.string(), params.newNamespace, outputDir.string());

        return readOutputFile(outputDir / inputFile.filename());
    }

    void
    runExecutable(std::string const& inputFile, std::string const& newNamespace, std::string const& outputDir)
    {
        const std::string command = std::string(EXE_PATH) + " --cppms_src=" + inputFile + " --namespace=" + newNamespace
                                    + " --namespace-alias " + outputDir;

        std::cout << "Test command: " << command << std::endl;
        const int result = std::system(command.c_str());
        if (result != 0)
        {
            throw std::runtime_error("Executable failed: " + std::to_string(result));
        }
    }

    std::string
    readOutputFile(fs::path const& outputFile)
    {
        std::ifstream ifs(outputFile);
        return std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    }

  private:
    fs::path
    createInputFile(ReplacerParams const& params)
    {
        fs::path inputFile;
        if (params.fileType == "json")
        {
            inputFile = tempDir / "manifest.json";
        }
        else if (params.fileType == "config")
        {
            inputFile = tempDir / "config.h.in";
        }
        else
        {
            inputFile = tempDir / "input.cpp";
        }

        std::ofstream ofs(inputFile);
        ofs << params.input;
        ofs.close();
        return inputFile;
    }
};

// Tests basic namespace declaration replacement with alias
TEST_F(NamespaceReplacerTest, ReplacesNamespaceDeclaration)
{
    const ReplacerParams params = {R"(
namespace cppmicroservices {
    void someFunction() {
        // Some code here
    }
}
)",
                                   "newnamespace"};
    const std::string expected  = R"(
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
    void someFunction() {
        // Some code here
    }
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests using namespace statement replacement
TEST_F(NamespaceReplacerTest, ReplacesUsingNamespaceDeclaration)
{
    const ReplacerParams params = {R"(
using namespace cppmicroservices;

void someFunction() {
    // Some code here
}
)",
                                   "newnamespace"};
    const std::string expected  = R"(
using namespace newnamespace;

void someFunction() {
    // Some code here
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests namespace replacement in template parameters
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInTemplateParameters)
{
    const ReplacerParams params = {R"(
std::vector<cppmicroservices::ServiceReference> serviceRefs;
std::map<cppmicroservices::Bundle, cppmicroservices::framework::BundleContext> bundleContextMap;
)",
                                   "newnamespace"};
    const std::string expected  = R"(
std::vector<newnamespace::ServiceReference> serviceRefs;
std::map<newnamespace::Bundle, newnamespace::framework::BundleContext> bundleContextMap;
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests namespace alias replacement
TEST_F(NamespaceReplacerTest, ReplacesNamespaceAlias)
{
    const ReplacerParams params = {"namespace alias = cppmicroservices;", "newnamespace"};
    const std::string expected  = "namespace alias = newnamespace;";
    const std::string result    = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests namespace replacement in preprocessor macros
TEST_F(NamespaceReplacerTest, ReplacesDefine)
{
    const ReplacerParams params = {"#define MACRO cppmicroservices", "newnamespace"};
    const std::string expected  = "#define MACRO newnamespace";
    const std::string result    = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests namespace replacement when enclosed in parentheses
TEST_F(NamespaceReplacerTest, ReplacesParenthesizedNamespace)
{
    const ReplacerParams params = {"void func(cppmicroservices) {}", "newnamespace"};
    const std::string expected  = "void func(newnamespace) {}";
    const std::string result    = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests namespace prefix replacement
TEST_F(NamespaceReplacerTest, ReplacesNamespacePrefix)
{
    const ReplacerParams params = {"cppmicroservices::Bundle bundle;", "newnamespace"};
    const std::string expected  = "newnamespace::Bundle bundle;";
    const std::string result    = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests error handling for invalid inputs
TEST_F(NamespaceReplacerTest, ThrowsExceptionOnInvalidInput)
{
    // Test missing --cppms_src flag
    EXPECT_THROW({ runExecutable("", NEW_NAMESPACE, tempDir.string()); }, std::runtime_error);

    // Test missing --namespace flag
    EXPECT_THROW({ runExecutable(tempDir.string(), "", tempDir.string()); }, std::runtime_error);

    // Test missing output directory
    EXPECT_THROW({ runExecutable(tempDir.string(), NEW_NAMESPACE, ""); }, std::runtime_error);

    // Test non-existent input directory
    EXPECT_THROW({ runExecutable("/non/existent/path", NEW_NAMESPACE, tempDir.string()); }, std::runtime_error);

    // Test non-existent output directory
    EXPECT_THROW({ runExecutable(tempDir.string(), NEW_NAMESPACE, "/non/existent/path"); }, std::runtime_error);
}

// Tests namespace replacement in manifest.json files
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInManifestJson)
{
    const ReplacerParams params = {R"(
{
  "bundle.symbolic_name" : "cppmicroservices::example",
  "bundle.activator" : true,
  "bundle.name" : "Example Plugin",
  "bundle.description" : "A plugin using cppmicroservices framework"
}
)",
                                   "newnamespace",
                                   "json"};
    const std::string expected  = R"(
{
  "bundle.symbolic_name" : "newnamespace::example",
  "bundle.activator" : true,
  "bundle.name" : "Example Plugin",
  "bundle.description" : "A plugin using cppmicroservices framework"
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests namespace replacement in configuration header files
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInConfigHIn)
{
    const ReplacerParams params = {R"(
namespace cppmicroservices {
  // Some configuration code here
}
)",
                                   "newnamespace",
                                   "config"};
    const std::string expected  = R"(
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
  // Some configuration code here
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests handling of command-line arguments
TEST_F(NamespaceReplacerTest, HandlesHelpFlag)
{
    const std::string exePath(EXE_PATH);

    // Test help flags
    EXPECT_EQ(std::system((exePath + " -h").c_str()), 0);
    EXPECT_EQ(std::system((exePath + " --help").c_str()), 0);

    // Test No arguments passed
    EXPECT_EQ(std::system((exePath).c_str()), 0);

    // Test null argument
    EXPECT_NE(std::system((exePath + " --cppms_src").c_str()), 0);
}

// Tests handling of Invalid arguments
TEST_F(NamespaceReplacerTest, HandlesInvalidArgument)
{
    const std::string exePath(EXE_PATH);

    EXPECT_NE(std::system((exePath + " -invalidArg").c_str()), 0);
    EXPECT_NE(std::system((exePath + " --invalidArg").c_str()), 0);
}

// Tests handling of README files - preserves content without namespace changes
TEST_F(NamespaceReplacerTest, HandlesReadmeFiles)
{
    // Create a README.md file in the source directory
    fs::path readmeFile        = tempDir / "README.md";
    std::string initialContent = R"(
    # CppMicroServices Project

    ## Overview
    The CppMicroServices project provides a dynamic service registry and bundle system.

    ## Code Examples
    Using the cppmicroservices namespace:
    ```cpp
    #include <cppmicroservices/Bundle.h>

    namespace cppmicroservices {
        void exampleFunction() {
            Bundle bundle;
            // Example code
        }
    }
    )";

    std::ofstream(readmeFile) << initialContent;

    const fs::path outputDir = tempDir / "output";
    fs::create_directories(outputDir);

    runExecutable(tempDir.string(), NEW_NAMESPACE, outputDir.string());

    const fs::path outputReadmeFile = outputDir / "README.md";
    EXPECT_TRUE(fs::exists(outputReadmeFile));
    EXPECT_EQ(readOutputFile(outputReadmeFile), initialContent);
}

// Tests handling of tools/change_namespace directory - should be excluded
TEST_F(NamespaceReplacerTest, HandlesToolsChangeNamespaceFiles)
{

    // Create directory structure for tools/change_namespace
    fs::path toolsDir = tempDir / "tools" / "change_namespace";
    fs::create_directories(toolsDir);

    // Create a test file in tools/change_namespace directory
    const std::string toolContent = R"(
namespace cppmicroservices {
    void toolFunction() {}
})";

    std::ofstream(toolsDir / "tool.cpp") << toolContent;

    // Create output directory
    fs::path outputDir = tempDir / "output";
    fs::create_directories(outputDir);

    runExecutable(tempDir.string(), NEW_NAMESPACE, outputDir.string());

    // Verify the tool file was NOT copied
    fs::path outputToolFile = outputDir / "tools" / "change_namespace" / "tool.cpp";
    EXPECT_FALSE(fs::exists(outputToolFile));
}

// Tests handling of duplicate dependency paths
TEST_F(NamespaceReplacerTest, ProcessesDuplicateDependencyPaths)
{
    const fs::path nestedDir = tempDir / "nested";
    fs::create_directories(nestedDir);

    const std::string commonHeader = R"(
namespace cppmicroservices {
    void commonFunc() {}
})";

    const std::string sourceFile1 = R"(
#include "common.hpp"
namespace cppmicroservices {
    void func1() {}
})";

    const std::string sourceFile2 = R"(
#include "common.hpp"
namespace cppmicroservices {
    void func2() {}
})";

    std::ofstream(nestedDir / "common.hpp") << commonHeader;
    std::ofstream(tempDir / "main1.cpp") << sourceFile1;
    std::ofstream(tempDir / "main2.cpp") << sourceFile2;

    const ReplacerParams params {tempDir.string(), NEW_NAMESPACE};
    runReplacer(params);

    const fs::path outputDir = tempDir / "output";

    // Verify file existence
    EXPECT_TRUE(fs::exists(outputDir / "nested" / "common.hpp"));
    EXPECT_TRUE(fs::exists(outputDir / "main1.cpp"));
    EXPECT_TRUE(fs::exists(outputDir / "main2.cpp"));

    // Verify file contents
    const std::string expectedHeader = R"(
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
    void commonFunc() {}
})";

    const std::string expectedSource1 = R"(
#include "common.hpp"
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
    void func1() {}
})";

    const std::string expectedSource2 = R"(
#include "common.hpp"
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
    void func2() {}
})";

    EXPECT_EQ(readOutputFile(outputDir / "nested" / "common.hpp"), expectedHeader);
    EXPECT_EQ(readOutputFile(outputDir / "main1.cpp"), expectedSource1);
    EXPECT_EQ(readOutputFile(outputDir / "main2.cpp"), expectedSource2);
}

// Tests overwriting existing files in output directory
TEST_F(NamespaceReplacerTest, ReplacesExistingFileInOutputDir)
{
    const fs::path outputDir = tempDir / "output";
    fs::create_directories(outputDir);

    const std::string initialContent = R"(
namespace cppmicroservices {
    void oldFunction() {}
})";

    const std::string newContent = R"(
namespace cppmicroservices {
    void newFunction() {}
})";

    const std::string expectedContent = R"(
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
    void newFunction() {}
})";

    std::ofstream(outputDir / "input.cpp") << initialContent;

    const ReplacerParams params {newContent, NEW_NAMESPACE};
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expectedContent);
}

// Tests handling of required command line options
TEST_F(NamespaceReplacerTest, RequiresNamespaceOption)
{
    const std::string command = std::string(EXE_PATH) + " --cppms_src=" + tempDir.string() + " " + tempDir.string();

    EXPECT_NE(std::system(command.c_str()), 0);
}

// Tests namespace replacement without alias flag
TEST_F(NamespaceReplacerTest, ReplacesNamespaceWithoutAliasFlag)
{
    const fs::path inputFile  = tempDir / "input.cpp";
    const std::string content = R"(
namespace cppmicroservices {
    void testFunction() {
        // Some code here
    }
})";
    std::ofstream(inputFile) << content;

    const fs::path outputDir = tempDir / "output";
    fs::create_directories(outputDir);

    // Execute without the --namespace-alias flag
    const std::string command = std::string(EXE_PATH) + " --cppms_src=" + tempDir.string()
                                + " --namespace=" + NEW_NAMESPACE + " " + outputDir.string();

    const int result = std::system(command.c_str());
    EXPECT_EQ(result, 0);

    const std::string expected = R"(
namespace newnamespace {
    void testFunction() {
        // Some code here
    }
})";

    const std::string output = readOutputFile(outputDir / "input.cpp");
    EXPECT_EQ(output, expected);
}
