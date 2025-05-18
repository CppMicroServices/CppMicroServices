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

    static constexpr const char* NEW_NAMESPACE = "newnamespace";
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
        return readOutputFile(outputDir / inputFile.filename());
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

    std::string
    readOutputFile(fs::path const& outputFile)
    {
        std::ifstream ifs(outputFile);
        std::string output((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        ifs.close();

        return output;}
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
                                    NEW_NAMESPACE ,
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
                                    NEW_NAMESPACE ,
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
                                    NEW_NAMESPACE ,
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
        = { "namespace alias = cppmicroservices;", NEW_NAMESPACE , "input.cpp", inputDir, outputDir };

    const std::string expected = "namespace alias = newnamespace;";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces in #define macros
TEST_F(NamespaceReplacerTest, ReplacesDefine)
{
    const ReplacerParams params
        = { "#define MACRO cppmicroservices", NEW_NAMESPACE , "input.cpp", inputDir, outputDir };

    const std::string expected = "#define MACRO newnamespace";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespaces enclosed in parentheses
TEST_F(NamespaceReplacerTest, ReplacesParenthesizedNamespace)
{
    const ReplacerParams params
        = { "void func(cppmicroservices) {}", NEW_NAMESPACE , "input.cpp", inputDir, outputDir };

    const std::string expected = "void func(newnamespace) {}";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests replacement of namespace prefixes
TEST_F(NamespaceReplacerTest, ReplacesNamespacePrefix)
{
    const ReplacerParams params
        = { "cppmicroservices::Bundle bundle;", NEW_NAMESPACE , "input.cpp", inputDir, outputDir };

    const std::string expected = "newnamespace::Bundle bundle;";
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expected);
}

// Tests error handling for invalid inputs
TEST_F(NamespaceReplacerTest, ThrowsExceptionOnInvalidInput)
{
    // Test missing --cppms flag
    const ReplacerParams p1 = { "", NEW_NAMESPACE , "input.cpp", fs::path(""), outputDir };
    EXPECT_THROW({ runExecutable(p1); }, std::runtime_error);

    // Test missing --namespace flag
    const ReplacerParams p2 = { "", "", "input.cpp", inputDir, outputDir };
    EXPECT_THROW({ runExecutable(p2); }, std::runtime_error);

    // Test missing output directory
    const ReplacerParams p3 = { "", NEW_NAMESPACE , "input.cpp", inputDir, fs::path("") };
    EXPECT_THROW({ runExecutable(p3); }, std::runtime_error);

    // Test non-existent input directory
    const ReplacerParams p4 = { "", NEW_NAMESPACE , "input.cpp", fs::path("/non/existent/path"), outputDir };
    EXPECT_THROW({ runExecutable(p4); }, std::runtime_error);

    // Test non-existent output directory
    const ReplacerParams p5 = { "", NEW_NAMESPACE , "input.cpp", inputDir, fs::path("/non/existent/path") };
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
                                    NEW_NAMESPACE ,
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
                                    NEW_NAMESPACE ,
                                    "config.h.in",
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
    const ReplacerParams params = { initialContent, NEW_NAMESPACE, "README.md",inputDir, outputDir };
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, initialContent);
}

// Tests handling of tools/change_namespace directory - should be excluded
TEST_F(NamespaceReplacerTest, HandlesToolsChangeNamespaceFiles)
{

    // Create directory structure for tools/change_namespace
    fs::path toolsDir = inputDir / "tools" / "change_namespace";
    fs::create_directories(toolsDir);

    // Create a test file in tools/change_namespace directory
    const std::string toolContent = R"(
namespace cppmicroservices {
    void toolFunction() {}
})";

    std::ofstream(toolsDir / "tool.cpp") << toolContent;

     const ReplacerParams params = { toolContent, NEW_NAMESPACE, "tools.cpp", inputDir, outputDir};

    runExecutable(params);

    // Verify the tool file was NOT copied
    fs::path outputToolFile = outputDir / "tools" / "change_namespace" / "tool.cpp";
    EXPECT_FALSE(fs::exists(outputToolFile));
}

// Tests handling of required command line options omittion
TEST_F(NamespaceReplacerTest, RequiresNamespaceOption)
{
    // Test case where --namespace is completely omitted
    const std::string commndNamespaceMissing = std::string(EXE_PATH) + " --cppms_src=" + inputDir.string() + " " + outputDir.string();

    EXPECT_NE(std::system(commndNamespaceMissing.c_str()), 0);

    // Test case where --cppms_src is completely omitted
    const std::string commandCppms_srcMissing = std::string(EXE_PATH) + " --namespace=" + NEW_NAMESPACE + " " + outputDir.string();

    EXPECT_NE(std::system(commandCppms_srcMissing.c_str()), 0);
}

// Tests namespace replacement without alias flag
TEST_F(NamespaceReplacerTest, ReplacesNamespaceWithoutAliasFlag)
{
    const fs::path inputFile  = inputDir / "input.cpp";
    const std::string content = R"(
namespace cppmicroservices {
    void testFunction() {
        // Some code here
    }
})";
    std::ofstream(inputFile) << content;


    // Execute without the --namespace-alias flag
    const std::string command = std::string(EXE_PATH) + " --cppms_src=" + inputDir.string()
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

// Tests overwriting existing files in output directory
TEST_F(NamespaceReplacerTest, ReplacesExistingFileInOutputDir)
{
    const fs::path outputDir = inputDir / "output";
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

    const ReplacerParams params {newContent, NEW_NAMESPACE, "input.cpp", inputDir, outputDir};
    const std::string result = runReplacer(params);
    EXPECT_EQ(result, expectedContent);
}

