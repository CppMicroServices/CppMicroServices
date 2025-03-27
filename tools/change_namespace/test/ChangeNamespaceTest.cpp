#include "CNTestingConfig.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace fs = std::filesystem;

class NamespaceReplacerTest : public ::testing::Test
{
  protected:
    fs::path inputDir;  // Temporary directory for test files
    fs::path outputDir; // Temporary directory for destination of tool execution

    // Creates a temporary directory for each test
    void
    SetUp() override
    {
        // Create source directory
        inputDir = fs::temp_directory_path() / "change_namespace_test";
        fs::create_directories(inputDir);

        // Create output directory
        outputDir = inputDir / "output";
        fs::create_directories(outputDir);
    }

    // Cleans up the temporary directory after each test
    void
    TearDown() override
    {
        fs::remove_all(inputDir);
    }

    struct ReplacerParams
    {
        std::string fileContent;
        std::string newNamespace;
        std::string fileName = "input.cpp";
        fs::path sourcePath;
        fs::path destinationPath;
    };

    // Runs the namespace replacer on the given input
    // Creates temporary input file, executes the tool, and returns the result
    std::string
    runReplacer(ReplacerParams const& params)
    {
        // Create temporary input file
        fs::path inputFile = params.sourcePath / params.fileName;
        std::ofstream ofs(inputFile);
        ofs << params.fileContent;
        ofs.close();

        // Run the executable
        runExecutable(params);

        // Read the output file
        fs::path outputFile = outputDir / inputFile.filename();
        std::ifstream ifs(outputFile);
        std::string output((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        ifs.close();

        return output;
    }

    // Executes the namespace replacer tool with the given parameters
    // Throws an exception if the execution fails
    void
    runExecutable(ReplacerParams const& params)
    {
        const std::string command = std::string(EXE_PATH) + " --cppms_src=" + params.sourcePath.string()
                                    + " --namespace=" + params.newNamespace + " --namespace-alias "
                                    + params.destinationPath.string();
        int result = std::system(command.c_str());
        if (result != 0)
        {
            throw std::runtime_error("Executable failed with error code: " + std::to_string(result));
        }
    }
};

// Tests replacement of namespace declarations
TEST_F(NamespaceReplacerTest, ReplacesNamespaceDeclaration)
{
    const ReplacerParams params = { R"(
namespace cppmicroservices {
    void someFunction() {
        // Some code here
    }
}
)",
                                    "newnamespace",
                                    "input.cpp",
                                    inputDir,
                                    outputDir };
    const std::string expected = R"(
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
    void someFunction() {
        // Some code here
    }
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of using namespace declarations
TEST_F(NamespaceReplacerTest, ReplacesUsingNamespaceDeclaration)
{
    const ReplacerParams params = { R"(
using namespace cppmicroservices;

void someFunction() {
    // Some code here
}
)",
                                    "newnamespace",
                                    "input.cpp",
                                    inputDir,
                                    outputDir };
    const std::string expected = R"(
using namespace newnamespace;

void someFunction() {
    // Some code here
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces in template parameters
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInTemplateParameters)
{
    const ReplacerParams params = { R"(
std::vector<cppmicroservices::ServiceReference> serviceRefs;
std::map<cppmicroservices::Bundle, cppmicroservices::framework::BundleContext> bundleContextMap;
)",
                                    "newnamespace",
                                    "input.cpp",
                                    inputDir,
                                    outputDir };
    const std::string expected = R"(
std::vector<newnamespace::ServiceReference> serviceRefs;
std::map<newnamespace::Bundle, newnamespace::framework::BundleContext> bundleContextMap;
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespace aliases
TEST_F(NamespaceReplacerTest, ReplacesNamespaceAlias)
{
    const ReplacerParams params
        = { "namespace alias = cppmicroservices;", "newnamespace", "input.cpp", inputDir, outputDir };
    const std::string expected = "namespace alias = newnamespace;";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces in #define macros
TEST_F(NamespaceReplacerTest, ReplacesDefine)
{
    const ReplacerParams params
        = { "#define MACRO cppmicroservices", "newnamespace", "input.cpp", inputDir, outputDir };
    const std::string expected = "#define MACRO newnamespace";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces enclosed in parentheses
TEST_F(NamespaceReplacerTest, ReplacesParenthesizedNamespace)
{
    const ReplacerParams params
        = { "void func(cppmicroservices) {}", "newnamespace", "input.cpp", inputDir, outputDir };
    const std::string expected = "void func(newnamespace) {}";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespace prefixes
TEST_F(NamespaceReplacerTest, ReplacesNamespacePrefix)
{
    const ReplacerParams params
        = { "cppmicroservices::Bundle bundle;", "newnamespace", "input.cpp", inputDir, outputDir };
    const std::string expected = "newnamespace::Bundle bundle;";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests error handling for invalid inputs
TEST_F(NamespaceReplacerTest, ThrowsExceptionOnInvalidInput)
{
    // Test missing --cppms flag
    const ReplacerParams p1 = { "", "newns", "input.cpp", fs::path(""), outputDir };
    EXPECT_THROW({ runExecutable(p1); }, std::runtime_error);

    // Test missing --namespace flag
    const ReplacerParams p2 = { "", "", "input.cpp", inputDir, outputDir };
    EXPECT_THROW({ runExecutable(p2); }, std::runtime_error);

    // Test missing output directory
    const ReplacerParams p3 = { "", "newns", "input.cpp", inputDir, fs::path("") };
    EXPECT_THROW({ runExecutable(p3); }, std::runtime_error);

    // Test non-existent input directory
    const ReplacerParams p4 = { "", "newns", "input.cpp", fs::path("/non/existent/path"), outputDir };
    EXPECT_THROW({ runExecutable(p4); }, std::runtime_error);

    // Test non-existent output directory
    const ReplacerParams p5 = { "", "newns", "input.cpp", inputDir, fs::path("/non/existent/path") };
    EXPECT_THROW({ runExecutable(p5); }, std::runtime_error);
}

// Tests replacement of namespaces in manifest.json files
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInManifestJson)
{
    const ReplacerParams params = { R"(
{
  "bundle.symbolic_name" : "cppmicroservices::example",
  "bundle.activator" : true,
  "bundle.name" : "Example Plugin",
  "bundle.description" : "A plugin using cppmicroservices framework"
}
)",
                                    "newnamespace",
                                    "manifest.json",
                                    inputDir,
                                    outputDir };
    const std::string expected = R"(
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

// Tests replacement of namespaces in config.h.in files
TEST_F(NamespaceReplacerTest, ReplacesNamespaceInConfigHIn)
{
    const ReplacerParams params = { R"(
namespace cppmicroservices {
  // Some configuration code here
}
)",
                                    "newnamespace",
                                    "config.in.h",
                                    inputDir,
                                    outputDir };
    const std::string expected = R"(
namespace newnamespace {} namespace cppmicroservices = newnamespace; namespace newnamespace {
  // Some configuration code here
}
)";

    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}
